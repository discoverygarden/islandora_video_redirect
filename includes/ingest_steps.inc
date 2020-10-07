<?php

/**
 * @file
 * Ingest step callbacks for Islandora Video Redirect.
 */

/**
 * Constructs an OBJ datastream pointing to an external video.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   The form to be rendered within Drupal.
 */
function islandora_video_redirect_create_obj_form(array $form, array &$form_state) {
  $form = array();

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#description' => t('URL where the video can be found.'),
    '#required' => TRUE,
    '#element_validate' => array('islandora_video_redirect_validate_redirect_url'),
  );

  return $form;
}

/**
 * Validates the URL before attempting to set.
 *
 * @param array $element
 *   The Drupal element that is being validated.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param array $form
 *   An array representing a form within Drupal.
 */
function islandora_video_redirect_validate_redirect_url(array $element, array &$form_state, array $form) {
  if (!valid_url($element['#value'], TRUE)) {
    form_error($element, t('The specified URL (@url) is not valid.', array(
      '@url' => $element['#value'],
    )));
  }
}

/**
 * Submit hanlder that creates the redirect OBJ datastream.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_video_redirect_create_obj_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  module_load_include('inc', 'islandora_video_redirect', 'includes/utilities');

  $object = islandora_ingest_form_get_object($form_state);

  $url = $form_state['values']['url'];

  // If this is an oEmbed URL construct the thumbnail upfront and avoid
  // derivatives.
  if ($oembed_data = islandora_video_redirect_retrieve_oembed_data($url)) {
    if (isset($oembed_data['thumbnail_url'])) {
      $tn_ds = $object->constructDatastream('TN', 'R');
      $tn_ds->url = $oembed_data['thumbnail_url'];
      $tn_ds->mimetype = file_get_mimetype($oembed_data['thumbnail_url']);
      $tn_ds->label = 'TN';
      $object->ingestdatastream($tn_ds);
    }
  }
  $ds = $object->constructDatastream('OBJ', 'R');
  $ds->url = $url;
  $ds->label = $oembed_data ? $oembed_data['title'] : pathinfo($url, PATHINFO_BASENAME);
  $ds->mimetype = $oembed_data ? 'text/xhtml' : file_get_mimetype($url);
  $object->ingestDatastream($ds);
}

/**
 * Undo submit handler that purges the redirect OBJ datastream.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_video_redirect_create_obj_form_undo_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');

  $object = islandora_ingest_form_get_object($form_state);
  $object->purgeDatastream('OBJ');
}
