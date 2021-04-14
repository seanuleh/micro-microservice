# micro-microservice
ESP32 Arduino Based MicroService

# About
Implementing the Developer API for the DPE API Shootout Redux... kind of

# Usage
1. Flash .ino file onto an esp32 dev board
2. Load postman collection and fire requests

Uses mDNS to advertise 'micromicro' hostname. Access at http://micromicro.local/

Examples:

Birth a Developer:
```
curl -X POST \
  http://micromicro.local/developers \
  -H 'content-type: application/json' \
  -d '{
  "id": "LX0Q26rQqq7knzc",
  "name": "Dev Devington",
  "team": "The A Team",
  "skils": [
    "C#"
  ],
  "createdAt": "2020-09-20T14:34:23+10:00",
  "updatedAt": "2020-09-20T16:00:00+10:00"
}'
```

Retrieve a Developer by ID:
```
curl -X GET http://micromicro.local/developers/LX0Q26rQqq7knzc
```

Shoot a Developer:
```
curl -X DELETE http://micromicro.local/developers/LX0Q26rQqq7knzc
```

# Gotchyas
Uses NVS for persistance, so has the following limitations:
1. Cannot search for developers (there is no scan utility for NVS)
2. ID's are limited to 15 characters


# Unit Tests
nope
