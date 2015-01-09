<?php

function calibre_library_permission() {
  return array(
    'calibre_library_view_catalog' => array(
      'title' => t('View Library Catalog'),
      'description' => 'View listings and metadata for books in the Library.',
    ),
    'calibre_library_download_books' => array(
      'title' => t('Download Library Books'),
      'description' => 'Download books from the Library collection.',
    ),
  );
}
