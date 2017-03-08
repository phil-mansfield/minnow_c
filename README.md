# minnow
A compression algorithm for cosmological N-body simulations

## Insallation

First, we need to download the code. To do this clone the `minnow` repository using the normal steps, and download libraries it depends on with the command
```
$ git submodule update --init --recursive
```
Next, we need to compile the code. This is done by `cd`ing into the `lz4/` directory and typing `make`. Afterwards, come back to this directoy and type `make` again. And you're done.
