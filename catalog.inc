<?php

function calibre_library_catalog_view() {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_title('Library Catalog');
  $content = array();

  $browse = array('#prefix' => '<h2>Browse</h2><ul>', '#suffix' => '</ul>');
  $browse[] = array( '#markup' => '<li>by <a href="/library/authors">Author</a>');
  $browse[] = array( '#markup' => '<li>by <a href="/library/series">Series</a>');
  $browse[] = array( '#markup' => '<li>by <a href="/library/tags">Tag</a>');
  $content[] = $browse;
  return $content;
}
