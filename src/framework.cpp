#include "framework.hpp"

#include <string>

namespace seworks::vrtx {
namespace {

constexpr char kSignalSource[] = R"luau(
-- COMPILED USING VELOCITY
-- ALL RIGHTS RESERVED
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
-- COMPILED USING VELOCITY
-- ALL RIGHTS RESERVED
local Signal = require(script.Parent.Signal)
local RunService = game:GetService("RunService")
local Players = game:GetService("Players")
local isServer = RunService:IsServer() or Players.LocalPlayer == nil

local Framework = {}
local services = {}
local serviceOrder = {}
local controllers = {}
local controllerOrder = {}
local clientServices = {}
local state = "registering"

local RemoteSignal = {}
RemoteSignal.__index = RemoteSignal

local function connectServerEvent(remote, callback)
	local okServerEvent, onServerEvent = pcall(function()
		return remote.OnServerEvent
	end)
	if okServerEvent and onServerEvent ~= nil then
		return onServerEvent:Connect(callback)
	end

	local ok, onRemoteEvent = pcall(function()
		return remote.OnRemoteEvent
	end)
	assert(ok and onRemoteEvent ~= nil, "RemoteEvent has no server event connection API")
	if type(onRemoteEvent) == "function" then
		return onRemoteEvent(remote, callback)
	end
	return onRemoteEvent:Connect(callback)
end

local function connectClientEvent(remote, callback)
	local okClientEvent, onClientEvent = pcall(function()
		return remote.OnClientEvent
	end)
	if okClientEvent and onClientEvent ~= nil then
		return onClientEvent:Connect(callback)
	end

	local ok, onRemoteEvent = pcall(function()
		return remote.OnRemoteEvent
	end)
	assert(ok and onRemoteEvent ~= nil, "RemoteEvent has no client event connection API")
	if type(onRemoteEvent) == "function" then
		return onRemoteEvent(remote, callback)
	end
	return onRemoteEvent:Connect(callback)
end

local function waitForClientEvent(remote)
	local okClientEvent, onClientEvent = pcall(function()
		return remote.OnClientEvent
	end)
	if okClientEvent and onClientEvent ~= nil then
		return onClientEvent:Wait()
	end

	local ok, onRemoteEvent = pcall(function()
		return remote.OnRemoteEvent
	end)
	assert(ok and onRemoteEvent ~= nil, "RemoteEvent has no client event wait API")
	if type(onRemoteEvent) == "function" then
		error("RemoteEvent OnRemoteEvent callback API cannot be used as Wait")
	end
	return onRemoteEvent:Wait()
end

local function bindServerFunction(remote, callback)
	local ok, onRemoteInvoke = pcall(function()
		return remote.OnRemoteInvoke
	end)
	if ok and onRemoteInvoke ~= nil then
		if type(onRemoteInvoke) == "function" then
			return onRemoteInvoke(remote, callback)
		end
		if onRemoteInvoke.Connect then
			return onRemoteInvoke:Connect(callback)
		end
	end

	local okServerInvoke = pcall(function()
		remote.OnServerInvoke = callback
	end)
	assert(okServerInvoke, "RemoteFunction has no server invoke API")
end

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
		return connectServerEvent(self._remote, callback)
	end
	return connectClientEvent(self._remote, callback)
end

function RemoteSignal:Wait()
	if self._local then
		return self._local:Wait()
	end
	assert(not isServer, "RemoteSignal:Wait can only be called by a client")
	assert(self._remote ~= nil, "RemoteSignal is not bound to a service")
	return waitForClientEvent(self._remote)
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

local function findUniqueChild(parent, name, className)
	local match
	for _, child in ipairs(parent:GetChildren()) do
		if child.Name == name then
			assert(child:IsA(className), "Expected " .. className .. " for " .. name)
			if match == nil then
				match = child
			else
				child:Destroy()
			end
		end
	end
	return match
end

local function bindClientEndpoints()
	local remoteRoot = createRemoteFolder()
	for _, service in ipairs(serviceOrder) do
		local serviceFolder = findUniqueChild(remoteRoot, service.Name, "Folder")
		if serviceFolder == nil then
			serviceFolder = Instance.new("Folder")
			serviceFolder.Name = service.Name
			serviceFolder.Parent = remoteRoot
		end

		for endpointName, endpoint in pairs(service.Client) do
			assert(type(endpointName) == "string" and endpointName ~= "", "Client endpoint names must be non-empty strings")
			if type(endpoint) == "function" then
				local remote = findUniqueChild(serviceFolder, endpointName, "RemoteFunction")
				if remote == nil then
					remote = Instance.new("RemoteFunction")
					remote.Name = endpointName
					remote.Parent = serviceFolder
				end
				bindServerFunction(remote, function(player, ...)
					return endpoint(service.Client, player, ...)
				end)
			elseif getmetatable(endpoint) == RemoteSignal then
				assert(not endpoint._bound, "A RemoteSignal cannot be registered more than once")
				local remote = findUniqueChild(serviceFolder, endpointName, "RemoteEvent")
				if remote == nil then
					remote = endpoint._remote
					remote.Name = endpointName
					remote.Parent = serviceFolder
				else
					endpoint._remote = remote
				end
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
									return connectClientEvent(remote, callback)
								end
							elseif method == "Wait" then
								return function()
									return waitForClientEvent(remote)
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

function Framework.CreateController(definition)
	assert(not isServer, "Controllers can only be created on the client")
	assert(state == "registering", "Controllers can only be created before Framework.Start()")
	assert(type(definition) == "table", "CreateController expects a table")
	assert(type(definition.Name) == "string" and definition.Name ~= "", "Controller requires a Name")
	assert(controllers[definition.Name] == nil, "Controller already exists: " .. definition.Name)

	controllers[definition.Name] = definition
	table.insert(controllerOrder, definition)
	return definition
end

function Framework.GetController(name)
	assert(not isServer, "Controllers can only be accessed on the client")
	local controller = controllers[name]
	assert(controller ~= nil, "Unknown controller: " .. tostring(name))
	return controller
end

function Framework.CreateSignal()
	return Signal.new()
end

function Framework.CreateRemoteSignal()
	assert(state == "registering", "RemoteSignals must be created before Framework.Start()")
	if not isServer then
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
		for _, controller in ipairs(controllerOrder) do
			if controller.Init then
				controller:Init()
			end
		end
		for _, service in ipairs(serviceOrder) do
			if service.Start then
				service:Start()
			end
		end
		for _, controller in ipairs(controllerOrder) do
			if controller.Start then
				controller:Start()
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