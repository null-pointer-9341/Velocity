#include "framework.hpp"

#include <string>

namespace seworks::vrtx {
namespace {

constexpr char kSignalSource[] = R"luau(
local Signal = {}
Signal.__index = Signal

local Connection = {}
Connection.__index = Connection

function Connection:Disconnect()
	if not self.Connected then
		return
	end

	self.Connected = false
	self._signal._connections[self] = nil
end

function Signal.new()
	return setmetatable({
		_connections = {},
		_destroyed = false,
	}, Signal)
end

function Signal:Connect(callback)
	assert(type(callback) == "function", "Signal:Connect expects a function")
	assert(not self._destroyed, "Cannot connect to a destroyed signal")

	local connection = setmetatable({
		Connected = true,
		_callback = callback,
		_signal = self,
	}, Connection)
	self._connections[connection] = true
	return connection
end

function Signal:Once(callback)
	local connection
	connection = self:Connect(function(...)
		connection:Disconnect()
		callback(...)
	end)
	return connection
end

function Signal:Fire(...)
	if self._destroyed then
		return
	end

	local listeners = {}
	for connection in pairs(self._connections) do
		table.insert(listeners, connection)
	end
	local arguments = table.pack(...)
	for _, connection in ipairs(listeners) do
		if connection.Connected then
			task.spawn(connection._callback, table.unpack(arguments, 1, arguments.n))
		end
	end
end

function Signal:Wait()
	local thread = coroutine.running()
	assert(thread ~= nil, "Signal:Wait must run in a coroutine")
	self:Once(function(...)
		task.spawn(thread, ...)
	end)
	return coroutine.yield()
end

function Signal:Destroy()
	if self._destroyed then
		return
	end

	self._destroyed = true
	for connection in pairs(self._connections) do
		connection:Disconnect()
	end
end

return Signal
)luau";

constexpr char kFrameworkSource[] = R"luau(
local Signal = require(script.Parent.Signal)
local RunService = game:GetService("RunService")
local isServer = RunService:IsServer()

local Framework = {}
local services = {}
local serviceOrder = {}
local clientServices = {}
local state = "registering"

local RemoteSignal = {}
RemoteSignal.__index = RemoteSignal

-- if a remote signal has _local set, its a client only one, so it just uses a normal signal and no RemoteEvent
-- (this is what fixes client only modules not finding their events)

function RemoteSignal:Fire(player, ...)
	if self._local then
		self._local:Fire(...)
		return
	end
	assert(isServer, "RemoteSignal:Fire can only be called by the server")
	assert(self._bound, "RemoteSignal is not bound to a service")
	self._remote:FireClient(player, ...)
end

function RemoteSignal:FireAll(...)
	if self._local then
		self._local:Fire(...)
		return
	end
	assert(isServer, "RemoteSignal:FireAll can only be called by the server")
	assert(self._bound, "RemoteSignal is not bound to a service")
	self._remote:FireAllClients(...)
end

function RemoteSignal:FireServer(...)
	if self._local then
		self._local:Fire(...)
		return
	end
	assert(not isServer, "RemoteSignal:FireServer can only be called by a client")
	assert(self._bound, "RemoteSignal is not bound to a service")
	self._remote:FireServer(...)
end

function RemoteSignal:Connect(callback)
	assert(type(callback) == "function", "RemoteSignal:Connect expects a function")
	if self._local then
		return self._local:Connect(callback)
	end
	assert(self._remote ~= nil, "RemoteSignal is not bound to a service")
	if isServer then
		return self._remote.OnServerEvent:Connect(callback)
	end
	return self._remote.OnClientEvent:Connect(callback)
end

function RemoteSignal:Wait()
	if self._local then
		return self._local:Wait()
	end
	assert(not isServer, "RemoteSignal:Wait can only be called by a client")
	assert(self._remote ~= nil, "RemoteSignal is not bound to a service")
	return self._remote.OnClientEvent:Wait()
end

local function createRemoteFolder()
	local folder = script.Parent:FindFirstChild("Remotes")
	if folder == nil then
		folder = Instance.new("Folder")
		folder.Name = "Remotes"
		folder.Parent = script.Parent
	end
	return folder
end

local function bindClientEndpoints()
	local remoteRoot = createRemoteFolder()
	for _, service in ipairs(serviceOrder) do
		local serviceFolder = Instance.new("Folder")
		serviceFolder.Name = service.Name
		serviceFolder.Parent = remoteRoot

		for endpointName, endpoint in pairs(service.Client) do
			assert(type(endpointName) == "string" and endpointName ~= "", "Client endpoint names must be non-empty strings")
			if type(endpoint) == "function" then
				local remote = Instance.new("RemoteFunction")
				remote.Name = endpointName
				remote.OnServerInvoke = function(player, ...)
					return endpoint(service.Client, player, ...)
				end
				remote.Parent = serviceFolder
			elseif getmetatable(endpoint) == RemoteSignal then
				assert(not endpoint._bound, "A RemoteSignal cannot be registered more than once")
				local remote = endpoint._remote
				remote.Name = endpointName
				remote.Parent = serviceFolder
				endpoint._bound = true
			else
				error("Client endpoint '" .. endpointName .. "' must be a function or RemoteSignal")
			end
		end
	end
end

function Framework.CreateService(definition)
	assert(state == "registering", "Services can only be created before Framework.Start()")
	assert(type(definition) == "table", "CreateService expects a table")
	assert(type(definition.Name) == "string" and definition.Name ~= "", "Service requires a Name")
	assert(services[definition.Name] == nil, "Service already exists: " .. definition.Name)

	definition.Client = definition.Client or {}
	assert(type(definition.Client) == "table", "Service Client must be a table")
	services[definition.Name] = definition
	table.insert(serviceOrder, definition)
	return definition
end

function Framework.GetService(name)
	if not isServer then
		-- services made on the client are local, so just hand them back right away
		-- (no waiting on the Remotes folder, the server never makes anything for these)
		if services[name] ~= nil then
			return services[name]
		end

		if clientServices[name] ~= nil then
			return clientServices[name]
		end

		local remoteRoot = script.Parent:WaitForChild("Remotes", 10)
		assert(remoteRoot ~= nil, "Velocity remotes were not started by the server")
		local serviceFolder = remoteRoot:WaitForChild(name, 10)
		assert(serviceFolder ~= nil, "Unknown service: " .. tostring(name))
		local clientService = {}
		setmetatable(clientService, {
			__index = function(self, endpointName)
				local remote = serviceFolder:WaitForChild(endpointName, 10)
				assert(remote ~= nil, "Unknown client endpoint: " .. tostring(name) .. "." .. tostring(endpointName))
				local endpoint
				if remote:IsA("RemoteFunction") then
					endpoint = function(_, ...)
						return remote:InvokeServer(...)
					end
				elseif remote:IsA("RemoteEvent") then
					endpoint = setmetatable({}, {
						__index = function(_, method)
							if method == "Connect" then
								return function(_, callback)
									return remote.OnClientEvent:Connect(callback)
								end
							elseif method == "Wait" then
								return function()
									return remote.OnClientEvent:Wait()
								end
							elseif method == "FireServer" then
								return function(_, ...)
									remote:FireServer(...)
								end
							end
						end,
					})
				else
					error("Unsupported client endpoint: " .. tostring(endpointName))
				end
				rawset(self, endpointName, endpoint)
				return endpoint
			end,
		})
		clientServices[name] = clientService
		return clientService
	end

	local service = services[name]
	assert(service ~= nil, "Unknown service: " .. tostring(name))
	return service
end

function Framework.CreateSignal()
	return Signal.new()
end

function Framework.CreateRemoteSignal()
	assert(state == "registering", "RemoteSignals must be created before Framework.Start()")
	if not isServer then
		-- on the client theres no RemoteEvent to make, so this is just a local signal pretending to be one
		return setmetatable({
			_local = Signal.new(),
			_bound = true,
		}, RemoteSignal)
	end
	return setmetatable({
		_remote = Instance.new("RemoteEvent"),
		_bound = false,
	}, RemoteSignal)
end

function Framework.Start()
	assert(state == "registering", "Framework.Start() can only be called once")
	state = "starting"

	local ok, failure = pcall(function()
		if isServer then
			bindClientEndpoints()
		end
		for _, service in ipairs(serviceOrder) do
			if service.Init then
				service:Init()
			end
		end
		for _, service in ipairs(serviceOrder) do
			if service.Start then
				service:Start()
			end
		end
	end)
	if not ok then
		state = "failed"
		error(failure, 2)
	end

	state = "started"
	return true
end

function Framework.IsStarted()
	return state == "started"
end

return Framework
)luau";

}  // namespace

void AddFrameworkModules(Scene& scene) {
	scene.AddInstance("Folder", "Velocity", "ReplicatedStorage");

	const auto framework = scene.AddInstance("ModuleScript", "Framework", "Velocity");
	scene.SetProperty(framework, "Source", std::string(kFrameworkSource));

	const auto signal = scene.AddInstance("ModuleScript", "Signal", "Velocity");
	scene.SetProperty(signal, "Source", std::string(kSignalSource));
}

}  // namespace seworks::vrtx