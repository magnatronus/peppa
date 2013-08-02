peppa
=====

Drupal 7 module exposing a REST API and providing JSON object storage

This module as 2 main external functions exposed via the REST API

1. It allows Drupal User verification and User account creation (this is configurable)
2. It allows CRUD operations on custom JSON objects

There is a minimal Administration function that can be found Administration->Configuration->Peppa REST API.

The module will install 3 module specific tables:

peppa_session  - this is used to track and validate a remote REST API user's session
peppa_object - this stores the JSON objects
peppa_apps - this is used to control the external application access to the REST API
-----------------------------------------------------------------------------

V2 of the API was created so that it made creating a Titanium Alloy Sync adapter
easier. The main change here was the way the object API returned and handled the
JSON objects.

-----------------------------------------------------------------------------
Please refer to www.spiralarm.co.uk/peppa for more info

