<div align="center">

[![GitHub Stars](https://img.shields.io/github/stars/emir12311/lnetx?style=for-the-badge)](https://github.com/emir12311/lnetx/stargazers)
[![GitHub Issues](https://img.shields.io/github/issues/emir12311/lnetx?style=for-the-badge)](https://github.com/emir12311/lnetx/issues)
[![License](https://img.shields.io/badge/license-GNU%20GPL%20v3-blue?style=for-the-badge)](https://www.gnu.org/licenses/gpl-3.0.html)

<br>

<img src="lnetx-logo.png" alt="lnetx logo" width="260">
	
<br>

<h1>lnetx</h1>

<p>A CLI for all your file transfer troubles.</p>

</div>

## What is lnetx?

`lnetx` is a CLI tool that sends a file from one machine to another on the same network.

The receiving side shows you the file's name, size and hash, and waits for your `y` before anything gets written.

## Why is lnetx?

I used LocalSend for a while, but its UI and discovery were finicky, and I just wanted something minimal.

That's why I built this tool.

## How is lnetx?

It was a nice project to work on, and it made me learn the struggles of networking.

If you want to see how it works internally, check out the [How It Works](#how-it-works) section.

## When is lnetx?

I was able to finish the project in around 2 weeks. Updates may come in the future.

## Installation

You can get the executable from the [Releases](https://github.com/emir12311/lnetx/releases) page, or you can build it yourself.

### Requirements

* Clang
* Git
* Make
* Linux

### Build from source

```bash
git clone https://github.com/emir12311/lnetx
cd lnetx
make
```

This will create a `lnetx` executable in `dist`.

You can move it somewhere in your `PATH`, or use it directly:

```bash
dist/lnetx --help
```

To remove the object files after building:

```bash
make clean
```

## Usage

The usage is pretty simple.

```bash
lnetx receive
```

Then, on the other machine:

```bash
lnetx send 192.168.1.42 myfile.zip
```

### Commands

| Command                    | Description                              |
| -------------------------- | ---------------------------------------- |
| `receive`                  | Start the receiver and wait for a file.  |
| `send <ip\|alias> <file>`  | Send a file to an IP or an alias.        |
| `alias create <name> <ip>` | Save a name for an IP.                   |
| `alias list`               | List your aliases.                       |

The receiver listens on TCP port `56389` and saves accepted files to `~/Downloads`.

## How It Works

1. The sender hashes the file and sends its name, size and BLAKE3 hash.
2. The receiver shows the metadata and asks you to confirm.
3. The file is sent in 64 KiB chunks and written as `<name>.part`.
4. The received file is hashed and compared with the sender's hash.
5. If the hashes match, the file is renamed into `~/Downloads`. If they don't, it is deleted.

In short:

```text
File
 │
 ├── Metadata ──→ name + size + BLAKE3 hash ──→ Receiver
 │
 └── 64 KiB chunks ──→ Transfer ──→ Write <name>.part
                                      │
                                      ├── Hash matches ──→ Rename → ~/Downloads
                                      │
                                      └── Hash differs ──→ Delete
```

That's basically it.

## License

This project is licensed under the **GNU General Public License v3.0**.

See the [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html) for the full license text.
