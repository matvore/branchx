#!/bin/sh

set -e

if [[ ! -d $HOME/.branchx ]]; then
	git clone https://github.com/matvore/branchx $HOME/.branchx
fi

if [[ ! -f $HOME/.branchx/bin/branchx ]]; then
	make -C $HOME/.branchx
fi

exec $HOME/.branchx/bin/branchx "$@"
