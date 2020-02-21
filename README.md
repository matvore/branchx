# branchx: `git branch` with a graph and easy commit selection

branchx is a simple extension to Git which serves two purposes:

 a. Make it easy to refer to a recent commit with only 3-4 keystrokes,
    regardless of repo size. This is done with ephemeral references of the form
    `h/##` which are generated whenever branchx is run.

 b. Give the user a view of all commits that have not been pushed back upstream
    yet. These commits are the most important, as they represent the commits
    which are still _in flux_ and may be rebased, amended, etc.

## Installation

Run:

```
$ curl -L https://raw.githubusercontent.com/google/branchx/master/wrapper.sh > binpath/commandname
```

Where `binpath` is some directory in $PATH, and `commandname` is whatever you
want the command to be named. Use a name like `git-foo` take make it a new Git
subcommand.

This requires GNU Make and a C compiler. On macOS, this means the Xcode Command
Line Tools should be installed.

## How to use

The default graph shown by `branchx` includes all local branches and (if
defined) all of their upstream branches. It also shows the most recent common
ancestor (i.e. the merge base) of all of these branches.

You can change the default branches shown by passing extra arguments. If any
arguments are passed, these are taken to be branches to show in the graph. The
merge base is also added to the graph automatically. For instance:

```
$ branchx master work
```

Shows the commits at the heads of `master` and `work`, and their most recent
common ancestor, and all commits in between.

## Notes

To set upstream of a branch: `$ git branch -u origin/master`
