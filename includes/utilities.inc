<?php

/**
 * @file
 * Utility functions for Islandora Video Redirect.
 */

/**
 * Determines if the URL is oEmbed compatible and returns the markup for it.
 *
 * @param string $url
 *   The video URL to look up.
 *
 * @return bool|string
 *   The oEmbed URL if it exists or FALSE if there's no match.
 */
function islandora_video_redirect_retrieve_oembed_url($url) {
  $providers = islandora_video_redirect_supported_providers();
  foreach ($providers as $provider) {
    // Iterate through each endpoint and see if there's a scheme that matches.
    foreach ($provider['endpoints'] as $endpoint) {
      foreach ($endpoint['schemes'] as $scheme) {
        // Convert scheme into a valid regular expression.
        $regexp = str_replace(['.', '*'], ['\.', '.*'], $scheme);
        if (preg_match("|^$regexp$|", $url)) {
          return format_string('!url?!query', array(
            '!url' => str_replace('{format}', 'json', $endpoint['url']),
            '!query' => drupal_http_build_query(array('url' => $url)),
          ));
        }
      }
    }
  }
  return FALSE;
}

/**
 * Helper to go get and parse the content to be embedded.
 *
 * @param string $url
 *   The source URL to look up.
 *
 * @return array|bool
 *   The array of data to be embedded, FALSE otherwise.
 */
function islandora_video_redirect_retrieve_oembed_data($url) {
  if ($oembed_url = islandora_video_redirect_retrieve_oembed_url($url)) {
    $response = drupal_http_request(stripslashes($oembed_url));
    if ($response->code === '200') {
      return json_decode($response->data, TRUE);
    }
  }
  return FALSE;
}

/**
 * Returns the supported oEmbed providers, for now just YouTube and Vimeo.
 */
function islandora_video_redirect_supported_providers() {
  return array(
    array(
      'provider_name' => 'YouTube',
      'provider_url' => 'https:\/\/www.youtube.com\/',
      'endpoints' => array(
        array(
          'schemes' => array(
            'https:\/\/*.youtube.com\/watch*',
            'https:\/\/*.youtube.com\/v\/*',
            'https:\/\/youtu.be\/*',
          ),
          'url' => "https:\/\/www.youtube.com\/oembed",
          'discovery' => TRUE,
        ),
      ),
    ),
    array(
      'provider_name' => 'YouTube',
      'provider_url' => 'https:\/\/vimeo.com\/',
      'endpoints' => array(
        array(
          'schemes' => array(
            'https:\/\/vimeo.com\/*',
            'https:\/\/vimeo.com\/album\/*\/video\/*',
            'https:\/\/vimeo.com\/channels\/*\/*',
            'https:\/\/vimeo.com\/groups\/*\/videos\/*',
            'https:\/\/vimeo.com\/ondemand\/*\/*',
            'https:\/\/player.vimeo.com\/video\/*',
          ),
          'url' => 'https:\/\/vimeo.com\/api\/oembed.{format}',
          'discovery' => TRUE,
        ),
      ),
    ),
  );
}
