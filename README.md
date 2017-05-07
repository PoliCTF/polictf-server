#PoliCTF Server daemons

This is the repository for the PoliCTF `registration` and `scoreboard` daemons.

Sample configuration files for daemons and NGINX as reverse proxy  can be found
in the `conf/` directory. Please thoroughly review config before deploying.

Suppose you want to run the `scoreboard` daemon on Debian/Ubuntu as user
`polictf-daemons` (set in the config) behind NGINX reverse proxy.
The following commands will have it up and running as a proper daemon:

    scoreboard -c /path/to/scoreabord.config
    chown polictf-daemons:www-data /path/to/scoreboard-fcgi-socket
    chmod 0660 /path/to/scoreboard-fcgi-socket

## Gameboard

The gameboard for the 2015 polictf is available here:
`https://github.com/PoliCTF/gameboard2015/tree/source`
(source branch)

## Scoreboard API
```
"/common/status", &scoreboard::commonstatus
"/common/challenge/([0-9]+)", &scoreboard::challenge
"/common/challenges", &scoreboard::challenges
"/team/status", &scoreboard::teamstatus
"/team/submit", &scoreboard::teamsubmit
"/login", &scoreboard::login
"/logout", &scoreboard::logout
```
