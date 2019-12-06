From https://github.com/yhirose/cpp-httplib/commit/d9479bc0b12e8a1e8bce2d34da4feeef488581f3

MIT License

===

cpp-httplib

A C++ single-file header-only cross platform HTTP/HTTPS library.

It's extremely easy to setup. Just include httplib.h file in your code!

© 2019 Yuji Hirose

===

- Additional changes to fit our needs:
- Removed url string encoding for ":"
- Add SSLVerifyMode enum
- Add the functions to Client/SSLClient:

```cpp
void set_verify(SSLVerifyMode mode);
void add_client_cert_ASN1(std::vector<unsigned char> cert, std::vector<unsigned char> key);
```
