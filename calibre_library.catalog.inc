<?php
module_load_include('inc', 'calibre_library', 'calibre_library.common');


function calibre_library_catalog_view() {
  drupal_set_title('Library Catalog');
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', ['group' => CSS_DEFAULT, 'every_page' => FALSE]);

  db_set_active('calibre');
  $books = [];
  foreach (db_query('SELECT b.id, b.title, date(b.pubdate) as pubdate, b.path, b.has_cover FROM {books} b ORDER BY b.timestamp DESC LIMIT 55', [], ['fetch' => PDO::FETCH_ASSOC]) as $book) {
    $book['authors'] = [];
    foreach (db_query('SELECT a.name FROM {books_authors_link} l, {authors} a WHERE l.book = :id AND l.author = a.id', [':id' => intval($book['id'])], ['fetch' => PDO::FETCH_ASSOC]) as $author) {
      $book['authors'][] = $author['name'];
    }
    $books[] = $book;
  }
  db_set_active();

  $content = ['#prefix' => '<table class="book-list"><tbody>', '#suffix' => '</tbody></table>'];
  foreach ($books as $book) {
    $title_parts = explode(':', $book['title'], 2);
    $pubdate = $book['pubdate'] ? new DateTime($book['pubdate']) : FALSE;
    $content[] = ['#markup' => sprintf('<tr><td class="image">%s</td><td class="title">%s%s%s</td><td class="pubdate">%s</td></tr>',
      $book['has_cover'] ? sprintf('<img src="%s">', file_create_url("public://calibre_library/{$book['path']}/cover.jpg")) : '',
      l(trim($title_parts[0]), "library/book/{$book['id']}"),
      count($title_parts) > 1 ? sprintf(': <span class="subtitle">%s</span>', trim($title_parts[1])) : '',
      count($book['authors']) ? sprintf('<div class="authors">by %s</div>', name_list_string($book['authors'], '&')) : '',
      $book['pubdate'] ? sprintf('<time datetime="%s">%s</time>', $pubdate->format('Y-m-d'), $pubdate->format('Y')) : ''
    )];
  }

  return $content;
}
