<?php
module_load_include('inc', 'calibre_library', 'calibre_library.common');


function calibre_library_series_list() {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', ['group' => CSS_DEFAULT, 'every_page' => FALSE]);
  drupal_set_title('Library Catalog');

  db_set_active('calibre');
  $series = [];
  foreach (db_query('SELECT s.id, s.name, count(l.book) AS books FROM {series} s, {books_series_link} l WHERE s.id = l.series GROUP BY s.id, s.name', [], ['fetch' => PDO::FETCH_ASSOC]) as $result) {
    $series[] = [
      'id' => $result['id'],
      'name' => $result['name'],
      'books' => $result['books'],
      'sort' => preg_replace( '/^(?:a|an|the) /i' , '' , $result['name']),
    ];
  }
  db_set_active();

  usort($series, function($a, $b) {return strcasecmp($a['sort'], $b['sort']);});
  $sections = [];
  foreach ($series as $x) {
    $initial = strtoupper(substr(trim($x['sort']), 0, 1));
    if (!array_key_exists($initial, $sections)) $sections[$initial] = ['#prefix' => sprintf('<h2>%s</h2><ul class="series-list">', $initial), '#suffix' => '</ul>'];
    $sections[$initial][] = ['#markup' => sprintf('<li>%s <span title="number of books in the collection">[%u]</span></li>',
      l(trim($x['name']), "library/series/{$x['id']}"),
      $x['books']
    )];
  }
  return $sections;
}


function calibre_library_series_view($id) {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', ['group' => CSS_DEFAULT, 'every_page' => FALSE]);
  drupal_set_breadcrumb([
    l(t('Home'), '<front>'),
    l(t('Library'), 'library'),
    l(t('Series'), 'library/series'),
  ]);
  db_set_active('calibre');
  $seriesName = trim(db_query('SELECT name FROM {series} WHERE id = :id', [':id' => intval($id)])->fetchField());
  drupal_set_title($seriesName);

  $authors = [];
  foreach (db_query('SELECT DISTINCT a.name FROM {books_series_link} bsl, {books_authors_link} bal, {authors} a WHERE bsl.series = :id AND bsl.book = bal.book AND bal.author = a.id', [':id' => intval($id)], ['fetch' => PDO::FETCH_ASSOC]) as $author) {
    $authors[] = trim(str_replace('|', ',', $author['name']));
  }

  $books = [];
  foreach (db_query('SELECT b.id, b.title, date(b.pubdate) as pubdate, b.series_index, b.path, b.has_cover FROM {books_series_link} l, {books} b WHERE l.series = :id AND l.book = b.id ORDER BY b.series_index', [':id' => intval($id)], ['fetch' => PDO::FETCH_ASSOC]) as $book) {
    $books[] = $book;
  }
  db_set_active();

  if (count($books) == 0) drupal_goto('library/series');
  if (count($books) == 1) drupal_goto(sprintf('library/book/%u', $books[0]['id']));

  $content = [];
  if (count($authors)) {
    $content[] = [
      '#prefix' => '<div class="book-authors">',
      '#markup' => sprintf('by %s', name_list_string($authors, '&')),
      '#suffix' => '</div>',
    ];
  }

  $list = ['#prefix' => '<table class="book-list"><tbody>', '#suffix' => '</tbody></table>'];
  foreach ($books as $book) {
    $title_parts = explode(':', $book['title'], 2);
    $pubdate = $book['pubdate'] ? new DateTime($book['pubdate']) : FALSE;
    $list[] = ['#markup' => sprintf('<tr><th>%s</th><td class="image">%s</td><td class="title">%s%s</td><td class="pubdate">%s</td></tr>',
      $book['series_index'] == intval($book['series_index']) ? intval($book['series_index']) : $book['series_index'],
      $book['has_cover'] ? sprintf('<img src="%s">', file_create_url("public://calibre_library/{$book['path']}/cover.jpg")) : '',
      l(trim($title_parts[0]), "library/book/{$book['id']}"),
      count($title_parts) > 1 ? sprintf(': <span class="subtitle">%s</span>', trim($title_parts[1])) : '',
      $book['pubdate'] ? sprintf('<time datetime="%s">%s</time>', $pubdate->format('Y-m-d'), $pubdate->format('Y')) : ''
    )];
  }
  $content[] = $list;

  return $content;
}
