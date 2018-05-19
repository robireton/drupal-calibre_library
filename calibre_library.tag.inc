<?php
module_load_include('inc', 'calibre_library', 'calibre_library.common');


function calibre_library_tags_list() {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', ['group' => CSS_DEFAULT, 'every_page' => FALSE]);
  drupal_set_title('Library Catalog');

  $sections = [];
  db_set_active('calibre');
  foreach (db_query('SELECT t.id, t.name, count(l.book) AS books FROM {tags} t, {books_tags_link} l WHERE t.id = l.tag GROUP BY t.id, t.name ORDER BY t.name', [], ['fetch' => PDO::FETCH_ASSOC]) as $tag) {
    $initial = strtoupper(substr(trim($tag['name']), 0, 1));
    if (!array_key_exists($initial, $sections)) $sections[$initial] = ['#prefix' => sprintf('<h2>%s</h2><ul class="tag-list">', $initial), '#suffix' => '</ul>'];
    $sections[$initial][] = ['#markup' => sprintf('<li>%s <span title="number of books in the collection">[%u]</span></li>',
      l(trim(str_replace('|', ',', $tag['name'])), "library/tag/{$tag['id']}"),
      $tag['books'])
    ];
  }
  db_set_active();

  return $sections;
}


function calibre_library_tag_view($id) {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', ['group' => CSS_DEFAULT, 'every_page' => FALSE]);
  drupal_set_breadcrumb([
    l(t('Home'), '<front>'),
    l(t('Library'), 'library'),
    l(t('Tags'), 'library/tags'),
  ]);
  db_set_active('calibre');
  $tagName = trim(str_replace('|', ',', db_query('SELECT name FROM {tags} WHERE id = :id', [':id' => intval($id)])->fetchField()));
  drupal_set_title($tagName);

  $books = [];
  foreach (db_query('SELECT b.id, b.title, date(b.pubdate) as pubdate, b.path, b.has_cover FROM {books_tags_link} l, {books} b WHERE l.tag = :id AND l.book = b.id ORDER BY b.pubdate DESC', [':id' => intval($id)], ['fetch' => PDO::FETCH_ASSOC]) as $book) {
    $book['authors'] = [];
    foreach (db_query('SELECT a.name FROM {books_authors_link} l, {authors} a WHERE l.book = :id AND l.author = a.id', [':id' => intval($book['id'])], ['fetch' => PDO::FETCH_ASSOC]) as $author) {
      $book['authors'][] = $author['name'];
    }
    $books[] = $book;
  }
  db_set_active();

  if (count($books) == 0) drupal_goto('library/tags');
  if (count($books) == 1) drupal_goto(sprintf('library/book/%u', $books[0]['id']));
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
