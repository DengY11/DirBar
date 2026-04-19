# DirBars

`DirBars` is a terminal tool, iterator directorys and output file size informattion:

output:
```text
./main.c                         3.6 KB  ######
./dir_info                      16.5 KB  ###############################
./README.md                      1.1 KB  ##
```


use cc to compile：

```sh
cc -Wall -Wextra -Werror -std=c11 main.c -o dirbars
```

## usage

```sh
./dirbars <path>
```

like

```sh
./dirbars .
```


