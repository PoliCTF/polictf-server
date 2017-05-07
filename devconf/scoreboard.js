{
  "service": {
    "api": "http",
    "port": 8089,
    "disable_xpowered_by": true,
    "allow-save-solved": true,
    "static-domain": "http://localhost:8080", //used for CORS
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
    "dbname": "polictf",
    "username": "polictf",
    "password": "polictf"
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

  "session": {
    "location": "client",
    "expire": "browser",

    "cookies": {
      "domain": "localhost",
      "prefix": "session",
      "secure": false
    },

    "client": {
      "cbc": "aes256",
      "cbc_key": "fe41f00cee442a16db4527d078acb0696945671eca8d957605c331549479b9f0",
      "hmac": "sha512",
      "hmac_key": "fe41f00cee442a16db4527d078acb0696945671eca8d957605c331549479b9f0"
    }
  },

  "security": {
    "display_error_message": true
  }
}

