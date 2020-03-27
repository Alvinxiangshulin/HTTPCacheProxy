# ECE568 HW2
The cache size is 50, using FIFO strategy. We handle the following HTTP `Cache-Control` header fields:
1. no-store
2. no-cache
3. max-age
4. max-stale
5. min-fresh
6. private
7. no `Cache-Control` in neither request and response

And also the `Expires` HTTP header field.

Note we do **not** clear proxy.log after shutting down the program. Instead we **append** to the existing log after restarting the program. (That is we use `std::ios::app`. If you happened to restart our program, you may find the last-time log still exists.)

You can use the following websites for testing:
## Test GET
**web url**: http://people.duke.edu/~bmr23/ece568/class.html

If not in cache, the log will be like the following (if `id = 1`):

*1: "GET http://people.duke.edu/~bmr23/ece568/class.html HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 19:44:41 2020*

*1: not in cache*

*1: Requesting "GET /~bmr23/ece568/class.html HTTP/1.1" from people.duke.edu*

*1: Received "HTTP/1.1 200 OK" from people.duke.edu*

*1: Responding "HTTP/1.1 200 OK"*

*1: cached, no cache-control found*

If the information is in cache, the log will be like the following (if `id` is 5):

*5: "GET http://people.duke.edu/~bmr23/ece568/class.html HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 19:44:44 2020*

*5: cached, but requires re-validation*

*5: Requesting "GET /~bmr23/ece568/class.html HTTP/1.1" from people.duke.edu*

*5: Received "HTTP/1.1 304 Not Modified" from people.duke.edu*

*5: Responding "HTTP/1.1 200 OK"*

In this case, the request contains `max-age=0`, therefore we perform re-validation. The response indicates "304 Not Modified", so we uses cached content to answer the client.

Similarly, you can try the following web sites:
1. http://pfister.ee.duke.edu/ (with picture on the website)
2. http://people.duke.edu/~bmr23/ece568/homework/hw1/hw1.pdf (large `Content-Length`)
3. http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx (`Transfer-Encoding: chunked`)

## Test POST
**web url**: http://httpbin.org/forms/post

The log will output the following (if `id = 2`):

*2: "POST http://httpbin.org/post HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:09:10 2020*

*2: Requesting "POST /post HTTP/1.1" from httpbin.org*

*2: Received "HTTP/1.1 200 OK" from httpbin.org*

*2: Responding "HTTP/1.1 200 OK"*

## Test CONNECT
**web url**: https://www.polyu.edu.hk

The log will output like the following:

*11: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:14 2020*

*11: Tunnel closed*

*9: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:14 2020*

*9: Tunnel closed*

*5: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:11 2020*

*5: Tunnel closed*

*16: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:17 2020*

*16: Tunnel closed*

*10: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:14 2020*

*10: Tunnel closed*

*8: "CONNECT www.polyu.edu.hk:443 HTTP/1.1" from 127.0.0.1 @ Fri Feb 28 20:34:14 2020*

*8: Tunnel closed*

You can use the following web sites for testing:
1. https://www.youtube.com/
2. https://www.wikipedia.org/
3. https://www.google.com/
