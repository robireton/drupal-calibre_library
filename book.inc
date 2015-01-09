<?php

function calibre_library_book_view($id) {
  $book = calibre_book_load($id);
  if (!$book['id']) drupal_goto('library');

  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Library'), 'library'),
  ));
  drupal_set_title($book['title']);
  return calibre_book_build_content($book);
}

function calibre_book_load($id) {
  db_set_active('calibre');

  $book = array(
    'id'            => NULL,
    'added'         => NULL,
    'title'         => NULL,
    'subtitle'      => NULL,
    'publisher'     => NULL,
    'pubdate'       => NULL,
    'series'        => NULL,
    'cover'         => NULL,
    'description'   => NULL,
    'authors'       => array(),
    'tags'          => array(),
    'identifiers'   => array(),
    'files'         => array(),
  );

  $series_index = NULL;
  $path = NULL;
  if ($record = db_query('SELECT b.id, b.title, datetime(b.timestamp) as added, date(b.pubdate) as pubdate, b.series_index, b.path, b.has_cover, c.text FROM {books} b LEFT JOIN {comments} c ON b.id = c.book WHERE b.id = :id', array(':id' => intval($id)))->fetchAssoc()) {
    $book['id'] = intval($record['id']);
    $title_parts = explode(':', $record['title'], 2);
    $book['title'] = trim($title_parts[0]);
    if (count($title_parts) > 1) $book['subtitle'] = trim($title_parts[1]);
    if ($record['added']) $book['added'] = new DateTime($record['added']);
    if ($record['pubdate']) $book['pubdate'] = new DateTime($record['pubdate']);
    $series_index = ($record['series_index'] == intval($record['series_index'])) ? intval($record['series_index']) : $record['series_index'];
    $book['text'] = $record['text'];
    $path = $record['path'];
    if ($record['has_cover']) $book['cover'] = "public://calibre_server/{$path}/cover.jpg";
  }

  if ($book['id']) {
    foreach (db_query('SELECT p.id, p.name FROM {books_publishers_link} bpl, {publishers} p WHERE bpl.book = :id AND bpl.publisher = p.id', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $publisher) {
      $book['publisher'] = array('id' => intval($publisher['id']), 'name' => trim($publisher['name']));
      break;
    }

    foreach (db_query('SELECT s.id, s.name FROM {books_series_link} bsl, {series} s WHERE bsl.book = :id AND bsl.series = s.id ORDER BY s.sort', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $series) {
      $book['series'] = array('id' => intval($series['id']), 'name' => trim($series['name']), 'index' => $series_index);
    }

    foreach (db_query('SELECT a.id, a.name FROM {books_authors_link} bal, {authors} a WHERE bal.book = :id AND bal.author = a.id', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $author) {
      $book['authors'][] = array('id' => intval($author['id']), 'name' => trim(str_replace('|', ',', $author['name'])));
    }

    foreach (db_query('SELECT t.id, t.name FROM {books_tags_link} btl, {tags} t WHERE btl.book = :id AND btl.tag = t.id ORDER BY t.name', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $tag) {
      $book['tags'][] = array('id' => intval($tag['id']), 'name' => trim(str_replace('|', ',', $tag['name'])));
    }

    foreach (db_query('SELECT type, val FROM {identifiers} WHERE book = :id ORDER BY type, val', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $identifiers) {
      $book['identifiers'][trim($identifiers['type'])] = trim($identifiers['val']);
    }

    foreach (db_query('SELECT format, name FROM {data} WHERE book = :id', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $file) {
      $book['files'][] = array('format' => $file['format'], 'uri' => sprintf('public://calibre_server/%s/%s.%s', $path, $file['name'], strtolower($file['format'])) );
    }
  }
  db_set_active();

  return $book;
}

function calibre_book_build_content($book, $view_mode = 'full') {
  $content = array();

  if ($book['subtitle']) {
    $content[] = array(
      '#prefix' => '<div class="book-subtitle">',
      '#markup' => $book['subtitle'],
      '#suffix' => '</div>',
    );
  }

  if (count($book['authors'])) {
    $content[] = array(
      '#prefix' => '<div class="book-authors">',
      '#markup' => sprintf('by %s', name_list_string(array_map(function($a) { return l($a['name'], "library/author/{$a['id']}"); }, $book['authors']), '&')),
      '#suffix' => '</div>',
    );
  }

  if ($book['cover'] && $view_mode != 'search_index') {
    $content[] = array(
      '#prefix' => '<div class="book-cover">',
      '#markup' => sprintf('<img src="%s">', file_create_url($book['cover'])),
      '#suffix' => '</div>',
    );
  }

  $metadata = array('#prefix' => '<table class="book-metadata"><tbody>', '#suffix' => '</tbody></table>');
  if ($book['series']) {
    $metadata[] = array( '#markup' => sprintf('<tr><th>series</th><td>%s · [%s]</td></tr>', l($book['series']['name'], "library/series/{$book['series']['id']}"), $book['series']['index']) );
  }

  if ($book['pubdate']) {
    $metadata[] = array( '#markup' => sprintf('<tr><th>originally published</th><td><time datetime="%s">%s</time></td></tr>', $book['pubdate']->format('Y-m-d'), $book['pubdate']->format('Y')) );
  }

  if (count($book['tags'])) {
    $metadata[] = array( '#markup' => sprintf('<tr><th>%s</th><td>%s</td></tr>', count($book['tags']) == 1 ? 'tag' : 'tags', implode(', ', array_map(function($a) { return l($a['name'], "library/tag/{$a['id']}"); }, $book['tags']))) );
  }

  foreach ($book['identifiers'] as $type => $val) {
    switch ($type) {
      case 'amazon':
      $metadata[] = array( '#markup' => sprintf('<tr><th>Amazon</th><td><a href="http://amzn.com/%s" target="_blank">%s</a></td></tr>', $val, $val) );
      break;

      case 'goodreads':
      $metadata[] = array( '#markup' => sprintf('<tr><th>Goodreads</th><td><a href="http://www.goodreads.com/book/show/%s" target="_blank">%s</a></td></tr>', $val, $val) );
      break;

      case 'google':
      $metadata[] = array( '#markup' => sprintf('<tr><th>Google</th><td><a href="http://books.google.com/books?id=%s" target="_blank">%s</a></td></tr>', $val, $val) );
      break;

      default:
      $metadata[] = array( '#markup' => sprintf('<tr><th>%s</th><td>%s</td></tr>', strtoupper($type), $val) );
    }
  }

  if ($book['added']) {
    $metadata[] = array( '#markup' => sprintf('<tr><th>added to collection</th><td><time datetime="%s">%s</time></td></tr>', $book['added']->format('c'), $book['added']->format('F Y')) );
  }

  $content[] = $metadata;

  if ($view_mode == 'full' && user_access('calibre_library_download_books')) {
    foreach ($book['files'] as $file) {
      $content[] = array(
        '#prefix' => '<div class="book-file">',
        '#markup' => l('Download ' . strtoupper($file['format']), file_create_url($file['uri'])),
        '#suffix' => '</div>',
      );
    }
  }

  if ($book['text']) {
    $content[] = array(
      '#prefix' => '<div class="book-description">',
      '#markup' => $book['text'],
      '#suffix' => '</div>',
    );
  }

  return $content;
}
