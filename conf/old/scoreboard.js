{
  "service": {
    "api": "fastcgi",
    "socket": "/tmp/scoreboard-fcgi-socket",
    "disable_xpowered_by": true,
    "allow-save-solved": true,
    "static-domain": "http://polictf.it",
    "rate-limit-seconds": 2
  },

  "http": {
      "script": "/scoreboard"
  },

  // Let NGINX handle it
  "gzip": {
      "enable": false
  },

  "server": {
    "mod": "thread",
    "threads": 40
  },

  "cache": {
    "backend": "thread_shared",
    "limit": 20000,
    "cacheable-header": "max-age=30, must-revalidate",
    "non-cacheable-header": "no-cache",
    "timeout": {
      "challengetext": 120,
      "challengesummary": 120,
      "commonstatus": 30,
      "globalwarnings": 10,
      "scores": 40,
      "teamalerts": 10,
      "teaminfo": 20,
      "teamsolutions": 20,
      "challengeid": 20,
      "challengesol": 20
    }
  },

  "db": {
    "hostname": "localhost",
    "dbname": "dbname",
    "username": "username",
    "password": "password"
  },

  "logging" : {
    "level" : "info",
    "syslog" : {
      "enable": false,
      "id" : "PoliCTF-scoreboard"
    },
    "file" : {
      "name" : "/var/log/polictf/scoreboard.log",
      "append" : false,
      "max_files": 16
    }
  },

  "daemon": {
    "enable": true,
    "user": "user",
    "group": "group",
    "lock": "/tmp/polictf-scoreboard.lock"
  },

  "session": {
    "location": "client",
    "expire": "browser",

    "cookies": {
      "domain": "domain",
      "prefix": "session",
      "secure": true
    },

    "client": {
      "cbc": "aes256",
      "cbc_key": "a very long and complex key, not like this one",
      "hmac": "sha512",
      "hmac_key": "a very long and complex key, not like this one"
    }
  }
}

