<?php

function calibre_library_series_list() {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_title('Series');

  db_set_active('calibre');
  $series = array();
  foreach (db_query('SELECT s.id, s.name, count(l.book) AS books FROM {series} s, {books_series_link} l WHERE s.id = l.series GROUP BY s.id, s.name', array(), array('fetch' => PDO::FETCH_ASSOC)) as $result) {
    $series[] = array(
      'id' => $result['id'],
      'name' => $result['name'],
      'books' => $result['books'],
      'sort' => preg_replace( '/^(?:a|an|the) /i' , '' , $result['name']),
    );
  }
  db_set_active();

  usort($series, function($a, $b) {return strcasecmp($a['sort'], $b['sort']);});
  $sections = array();
  foreach ($series as $x) {
    $initial = strtoupper(substr(trim($x['sort']), 0, 1));
    if (!array_key_exists($initial, $sections)) $sections[$initial] = array('#prefix' => sprintf('<h2>%s</h2><ul class="series-list">', $initial), '#suffix' => '</ul>');
    $sections[$initial][] = array( '#markup' => sprintf('<li><a href="/library/series/%u">%s</a> <span title="number of books in the collection">[%u]</span></li>',
      $x['id'],
      trim($x['name']),
      $x['books']
    ));
  }
  return $sections;
}

function calibre_library_series_view($id) {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Library'), 'library'),
    l(t('Series'), 'library/series'),
  ));
  db_set_active('calibre');
  $seriesName = trim(db_query('SELECT name FROM {series} WHERE id = :id', array(':id' => intval($id)))->fetchField());
  drupal_set_title($seriesName);

  $authors = array();
  foreach (db_query('SELECT DISTINCT a.name FROM {books_series_link} bsl, {books_authors_link} bal, {authors} a WHERE bsl.series = :id AND bsl.book = bal.book AND bal.author = a.id', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $author) {
    $authors[] = trim(str_replace('|', ',', $author['name']));
  }

  $books = array();
  foreach (db_query('SELECT b.id, b.title, date(b.pubdate) as pubdate, b.series_index, b.path, b.has_cover FROM {books_series_link} l, {books} b WHERE l.series = :id AND l.book = b.id ORDER BY b.series_index', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $book) {
    $books[] = $book;
  }
  db_set_active();

  if (count($books) == 0) drupal_goto('library/series');
  if (count($books) == 1) drupal_goto(sprintf('library/book/%u', $books[0]['id']));

  $content = array();
  if (count($authors)) {
    $content[] = array(
      '#prefix' => '<div class="book-authors">',
      '#markup' => sprintf('by %s', name_list_string($authors, '&')),
      '#suffix' => '</div>',
    );
  }

  $list = array('#prefix' => '<table class="book-list"><tbody>', '#suffix' => '</tbody></table>');
  foreach ($books as $book) {
    $title_parts = explode(':', $book['title'], 2);
    $pubdate = $book['pubdate'] ? new DateTime($book['pubdate']) : FALSE;
    $list[] = array('#markup' => sprintf('<tr><th>%s</th><td class="image">%s</td><td class="title">%s%s</td><td class="pubdate">%s</td></tr>',
      $book['series_index'] == intval($book['series_index']) ? intval($book['series_index']) : $book['series_index'],
      $book['has_cover'] ? sprintf('<img src="%s">', file_create_url("public://calibre_server/{$book['path']}/cover.jpg")) : '',
      sprintf('<a href="/library/book/%u">%s</a>', $book['id'], trim($title_parts[0])),
      count($title_parts) > 1 ? sprintf(': <span class="subtitle">%s</span>', trim($title_parts[1])) : '',
      $book['pubdate'] ? sprintf('<time datetime="%s">%s</time>', $pubdate->format('Y-m-d'), $pubdate->format('Y')) : ''
    ));
  }
  $content[] = $list;

  return $content;
}
