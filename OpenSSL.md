# OpenSSL

Im [SSL Shopper](https://www.sslshopper.com/article-most-common-openssl-commands.html) werden die meistgenutzten SSL Kommandos dargestellt.

## Zertifikat überprüfen

```
openssl x509 -in cert.pem -text -noout
```


## Self signed Zertifikat (one liner)

Kein existierender privater Schlüssel: 

```
openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout privkey.pem -out cert.pem -subj "/CN=cloud.innovationv2.localdomain" -days 390 -addext "subjectAltName=DNS:cloud.innovationv2.localdomain" -addext "extendedKeyUsage=serverAuth"
```

Ein bereits existierender privater Schlüssel (privkey.pem als Beispiels-Dateiname):

```
openssl req -x509 -sha256 -nodes -key privkey.pem -out cert.pem -subj "/CN=cloud.innovationv2.localdomain" -days 390 -addext "subjectAltName=DNS:cloud.innovationv2.localdomain;IP:192.168.1.151" -addext "extendedKeyUsage=serverAuth"
```


