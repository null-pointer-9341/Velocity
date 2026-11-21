# Velocity Framework Snippets

## Inspiration

This was heavily inspired by Knit's snippets.

## Building

In order to compile the snippet extension, you'll have to run:
```bash
npx @vscode/vsce package
```

Make sure you are inside of `/vscode-extension`.

Then, to install the generated VSIX you can run:
```bash
code --install-extension velocity-framework-snippets-1.1.0.vsix --force
```

## How to test

You can type a shortcut like `velservice` and see if it auto suggests it.

Or just check the [file](./snippets/velocity-framework.code-snippets) and read how it works.

Also, don't forget to reload the window after installation.

You can do that with `Ctrl+Shift+P` and typing `Reload Window`.

## License

Well I don't really care about this snippet, it's just something on the side to add to make the framework usage easier.

Use it in your own project, I couldn't care less about it.

## Thanks

Have fun using the compiler!