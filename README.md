# anope-modules

## m_webhooks

This module adds remote webhook support to Anope for certain events.

* ns_register
* ns_drop
* ns_group

### Dependencies

`m_webhooks` depends on the following external libraries (and their development headers):

* libcurl4

### Installing

To install m_webhooks:

* Place the `cpp` file inside $anope_source_dir$/modules/third/
* `cd $anope_source_dir$`
* ./Config
* make
* make install
* Add the module enabling code to your modules.conf

### Configuration

The following is the default config for this module

```
module
{
	name = "m_webhooks"
	webhook = ""

	ns_drop = "no"
	ns_register = "no"
	ns_group = "no"

	timeout = "5"
	connect_timeout = "5"
}
```

#### webhook

URL to the endpoint that will be called when an event is fired.

Example: *https://www.example.com/hook*

#### ns_drop, ns_register, ns_group

Toggles for events to hook, or not.

#### timeout

How long to wait for a response from the endpoint while calling it.

#### connect_timeout

How long to wait for the socket to connect to the endpoint.