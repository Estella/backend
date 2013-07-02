# Repsheet Backend  [![Build Status](https://secure.travis-ci.org/repsheet/backend.png)](http://travis-ci.org/repsheet/backend?branch=master)

This is the backend infrastructure for
[Repsheet](https://github.com/repsheet/repsheet). It provides
automatic processing of the current state of actors and allows for
automatic blacklisting of repeat offenders. It is designed to be run
under cron.

## Compiling

You need the [hiredis](https://github.com/redis/hiredis) library
installed for compilation and linking. You also need the standard
autotools packages. This includes `autoconf`, `automake`, and
`libtool`.

```sh
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

## Usage

If you pass no arugments to the repsheet binary, it will default to
simply scoring the actors in a sorted set inside of Redis under the
offenders key.

```sh
$ repsheet
No options specified, performing score operation.
To remove this message, specify -s (score) or [-r | -b] (report or blacklist)
```

If you specify `-s` the message will go away

```sh
$ repsheet
$
```

Using the `-r` option will print a report of the top ten unblacklisted actors

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

And the `-b` option will sweep the offenders list and blacklist any
offenders that have an offense count higher than the threshold `-t`
(default 200)

```sh
$ repsheet -b
repsheet -b -t 100
Blacklisting the following repeat offenders (threshold == 100)
  1.1.1.76
  1.1.1.43
  1.1.1.172
  1.1.1.79
  1.1.1.142
  1.1.1.198
  1.1.1.167
  1.1.1.48
  1.1.1.98
  1.1.1.105
  1.1.1.113
  1.1.1.50
  1.1.1.170
  1.1.1.189
  1.1.1.67
```
