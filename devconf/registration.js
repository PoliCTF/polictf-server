{
  "service": {
    "api": "fastcgi",
    "socket": "/tmp/registration-fcgi-socket",
    "disable_xpowered_by": true,
    "static-domain": "http://polictf.it"
  },

  "http": {
    "script_name": "/registration"
  },

  // Let NGINX handle it
  "gzip": {
      "enable": false
  },

  "cache": {
    "backend": "thread_shared",
    "cache.limit": 2000
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
      "id" : "PoliCTF-registration"
    },
    "file" : {
      "name" : "/var/log/polictf/registration.log",
      "append" : false,
      "max_files": 16
    }
  },

  "url": {
    "confirm-ok": "http://polictf.it/reg_ok.html",
    "confirm-ko": "http://polictf.it/reg_ko.html"
  },

  "daemon": {
    "enable": true,
    "user": "user",
    "group": "group",
    "lock": "/tmp/polictf-registration.lock"
  },

  "smtp": {
    "host": "localhost",
    "user": "user",
    "password": "password",
    "auth": 1,
  },

  "mail": {
    "subject": "[PoliCTF] Confirm registration",
    "text": "Hi !{name}!\n\nPlease confirm your registration by clicking the following link:\n\nhttps://registration.polictf.it/confirm/!{uid}",
    "from_address": "no-reply@polictf.it",
    "from_name": "PoliCTF shiny bot"
  }
}

