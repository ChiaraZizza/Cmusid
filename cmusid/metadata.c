#define _GNU_SOURCE

#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>

const char *ACOUSTID_API_URL = "http://api.acoustid.org/v2/lookup";
const char *APPLICATION_ID = "jif76R78Wd";

void
fetchMetadata (char *fingerprint, int duration)
{
  printf ("Fetching Metadata...\n | Duration: %d\n | Fingerprint: %s\n",
	  duration, fingerprint);
  CURL *curl = curl_easy_init ();
  if (curl)
    {
      CURLcode res;
      curl_easy_setopt (curl, CURLOPT_URL, ACOUSTID_API_URL);
      char *body;
      asprintf (&body, "client=%s&duration=%d&fingerprint=%s", APPLICATION_ID,
		duration, fingerprint);
      curl_easy_setopt (curl, CURLOPT_POSTFIELDS, body);

      res = curl_easy_perform (curl);
      if (res != CURLE_OK)
	fprintf (stderr, "curl_easy_perform() failed: %s\n",
		 curl_easy_strerror (res));
      curl_easy_cleanup (curl);
      free (body);
    }
  else
    {
      fprintf (stderr, "Error Initializing cURL");
    }
}

