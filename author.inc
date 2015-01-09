<?php

function calibre_library_authors_list() {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_title('Authors');

  $sections = array();
  db_set_active('calibre');
  foreach (db_query('SELECT a.id, a.name, a.sort, count(l.book) AS books FROM {authors} a, {books_authors_link} l WHERE a.id = l.author GROUP BY a.id, a.name ORDER BY a.sort', array(), array('fetch' => PDO::FETCH_ASSOC)) as $author) {
    $initial = strtoupper(substr(trim($author['sort']), 0, 1));
    if (!array_key_exists($initial, $sections)) $sections[$initial] = array('#prefix' => sprintf('<h2>%s</h2><ul class="author-list">', $initial), '#suffix' => '</ul>');
    $sections[$initial][] = array( '#markup' => sprintf('<li><a href="/library/author/%u">%s</a> <span title="number of books in the collection">[%u]</span></li>',
      $author['id'],
      trim(str_replace('|', ',', $author['name'])),
      $author['books'])
    );
  }
  db_set_active();

  return $sections;
}

function calibre_library_author_view($id) {
  drupal_add_css(drupal_get_path('module', 'calibre_library') . '/calibre_library.css', array('group' => CSS_DEFAULT, 'every_page' => FALSE));
  drupal_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Library'), 'library'),
    l(t('Authors'), 'library/authors'),
  ));
  db_set_active('calibre');
  $authorName = trim(str_replace('|', ',', db_query('SELECT name FROM {authors} WHERE id = :id', array(':id' => intval($id)))->fetchField()));
  drupal_set_title($authorName);

  $books = array();
  foreach (db_query('SELECT b.id, b.title, date(b.pubdate) as pubdate, b.path, b.has_cover FROM {books_authors_link} l, {books} b WHERE l.author = :id AND l.book = b.id ORDER BY b.pubdate DESC', array(':id' => intval($id)), array('fetch' => PDO::FETCH_ASSOC)) as $book) {
    $books[] = $book;
  }
  db_set_active();

  if (count($books) == 0) drupal_goto('library/authors');
  if (count($books) == 1) drupal_goto(sprintf('library/book/%u', $books[0]['id']));
  $content = array('#prefix' => '<table class="book-list"><tbody>', '#suffix' => '</tbody></table>');
  foreach ($books as $book) {
    $title_parts = explode(':', $book['title'], 2);
    $pubdate = $book['pubdate'] ? new DateTime($book['pubdate']) : FALSE;
    $content[] = array('#markup' => sprintf('<tr><td class="image">%s</td><td class="title">%s%s</td><td class="pubdate">%s</td></tr>',
      $book['has_cover'] ? sprintf('<img src="%s">', file_create_url("public://calibre_server/{$book['path']}/cover.jpg")) : '',
      sprintf('<a href="/library/book/%u">%s</a>', $book['id'], trim($title_parts[0])),
      count($title_parts) > 1 ? sprintf(': <span class="subtitle">%s</span>', trim($title_parts[1])) : '',
      $book['pubdate'] ? sprintf('<time datetime="%s">%s</time>', $pubdate->format('Y-m-d'), $pubdate->format('Y')) : ''
    ));
  }

  return $content;
}
