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
