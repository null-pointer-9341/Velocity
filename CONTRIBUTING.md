# Contribution guide

Thanks for taking your time to attempt collaboration towards `Velocity`.

## How to contribute

You can contribute in 2 ways:
    * [Submit a pull request](https://github.com/null-pointer-9341/Velocity/pulls) and fix the issue directly.
    * [File an issue](https://github.com/null-pointer-9341/Velocity/issues) for the bug.

### Submit a pull request

1. Fork the repository.

2. Clone the forked repository.
```bash
git clone https://github.com/null-pointer-9341/Velocity
cd Velocity
```

3. Create a branch for your patch.
```bash
git checkout -b fix/<ENTER-NAME>
```

4. Create and commit changes. (View the commit policy [below](./CONTRIBUTING.md)).

5. Push changes to your repository (fork) and file a Pull Request to the default branch (`development`), and write a detailed description covering the changes.

### File an issue

1. Check if the issue already exists in [issues](https://github.com/null-pointer-9341/Velocity/issues)

2. Create a new issue with a detailed name and optionally images/videos of the behavior or logs.

3. **Use labels** for easier organization. (compatibility, documentation, behavior, other).

### Commit policy

Use markdown headers for better readability.

Include code fences / inline code for any commands.

* If you're gonna use code fences, please use the full command.

All paths need to be [relative](https://desktop.arcgis.com/en/arcmap/latest/tools/supplement/pathnames-explained-absolute-relative-unc-and-url.htm).

Please keep pull requests serious and remember grammar.

### Pull request discussion policy

Please follow [no hello](https://nohello.net/en/), this is not required however reccomended for easier communication.

### Architecture

Store documentation resources in `docs/resources` (like images or videos) and follow [relative paths](https://desktop.arcgis.com/en/arcmap/latest/tools/supplement/pathnames-explained-absolute-relative-unc-and-url.htm).

### Check per pull request

Before you file the pull review, make sure:

* The pull request target repository is the default branch (`development`)
* The pull request description explains what, why, and any user-visible changes.
* Spelling and grammar is correct.
* Links and images render correctly.
* Any neccesary screenshots or examples are included.
* If the change affects a live site, the preview/build has been tested locally.
* Ensure all paths are relative.
* The pull requesst is relatively small and focused. (If change is large, consider splitting into multiple pull requests).

## After contribution

That's all!

Thank you for taking your time and collaborating towards `Velocity`!