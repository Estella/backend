# Repsheet Backend  [![Build Status](https://secure.travis-ci.org/repsheet/backend.png)](http://travis-ci.org/repsheet/backend?branch=master)

This is the backend infrastructure for
[Repsheet](https://github.com/repsheet/repsheet). It provides
automatic processing of the current state of actors and allows for
automatic blacklisting of repeat offenders. It is designed to be run
under cron.

## Compiling

You need the [hiredis](https://github.com/redis/hiredis),
[libcurl](http://curl.haxx.se/libcurl/), and
[libjson/json-c](https://github.com/json-c/json-c) libraries installed
for compilation and linking. You also need the standard autotools
packages. This includes `autoconf`, `automake`, and `libtool`.

```sh
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

## Usage

```
$ repsheet --version
Repsheet Backend Version 2.0.0
usage: repsheet [-srauv] [-h] [-p] [-e] [-t] [-o]
  --score                  -s score actors
  --report                 -r report top 10 offenders
  --analyze                -a analyze and act on offenders
  --publish                -u publish blacklist to upstream providers
  --host                   -h <redis host>
  --port                   -p <redis port>
  --expiry                 -e <redis expiry> blacklist expire time
  --modsecurity_threshold  -t <blacklist threshold>
  --ofdp_threshold         -o <ofdp threshold> score and blacklist actors against wafsec.com
  --version                -v print version and help
```

If you pass no arugments to the repsheet binary, it will default to
simply scoring the actors in a sorted set inside of Redis under the
offenders key.

```sh
$ repsheet
No options specified, performing score operation
```

The score `-s / --score` operation can be run on its own, but is also
run during the report and analyze operations.

Using the `-r / --report` option will print a report of the top ten
unblacklisted actors.

```sh
$ repsheet -r
Top 10 Suspsects (not yet blacklisted)
  1.1.1.1	20444 offenses
  1.1.1.215	996 offenses
  1.1.1.210	991 offenses
  1.1.1.55	986 offenses
  1.1.1.218	969 offenses
  1.1.1.156	964 offenses
  1.1.1.49	954 offenses
  1.1.1.200	948 offenses
  1.1.1.85	945 offenses
  1.1.1.21	943 offenses
  1.1.1.45	934 offenses
```

And the `-a / --analyze` option will analyze the offenders list and
blacklist any offenders that have an offense count higher than the
ModSecurity threshold `-t / --modsecurity_threshold` (default 200),
the OFDP threshold `-o / --ofdp_threshold` (default 50), or have been previously
blacklisted and have returned after their blacklist has expired.

```sh
$ repsheet --analyze --modsecurity_threshold 75 --ofdp_threshold 20
Actor 1.1.1.1 has been blacklisted: The actor has exceeded the ModSecurity blacklist threshold. [Score: 181]
```
