<?php

function calibre_library_search_info() {
  return array(
    'title' => 'Books',
    'path' => 'library',
    'conditions_callback' => 'callback_search_conditions',
  );
}

function calibre_library_update_index() {
  $limit = 5;
  // $limit = (int) variable_get('search_cron_limit', 100);

  $reindex = array();
  foreach (db_query("SELECT sid, reindex FROM {search_dataset} WHERE type = 'calibre_book'", array(), array('fetch' => PDO::FETCH_ASSOC)) as $result) {
    $reindex[intval($result['sid'])] = boolval($result['reindex']);
  }

  $bookIDs = array();
  db_set_active('calibre');
  foreach (db_query("SELECT id FROM {books} ORDER BY last_modified", array(), array('fetch' => PDO::FETCH_ASSOC)) as $result) {
    $id = intval($result['id']);
    if (!array_key_exists($id, $reindex) || $reindex[$id]) $bookIDs[] = $id;
    if (count($bookIDs) >= $limit) break;
  }
  db_set_active();

  foreach ($bookIDs as $id) {
    $book = calibre_book_load($id);
    $content = calibre_book_build_content($book, 'search_index');
    $html = drupal_render($content);
    $title = '<h1>' . check_plain($book['title']) . '</h1>';

    search_index($id, 'calibre_book', $title . $html);
  }

}

function calibre_library_search_execute($keys = NULL, $conditions = NULL) {
  // Build matching conditions
  $query = db_select('search_index', 'i', array('target' => 'slave'))->extend('SearchQuery')->extend('PagerDefault');
  //$query->join('node', 'n', 'n.nid = i.sid');
  $query->searchExpression($keys, 'calibre_book');

  // Insert special keywords.
  $query->setOption('type', 'calibre_book');
  //$query->setOption('language', 'n.language');
  //if ($query->setOption('term', 'ti.tid')) {
    //$query->join('taxonomy_index', 'ti', 'n.nid = ti.nid');
  //}
  // Only continue if the first pass query matches.
  if (!$query->executeFirstPass()) {
    return array();
  }

  // Add the ranking expressions.
  _node_rankings($query);

  // Load results.
  $find = $query->limit(10)->execute();

  $results = array();

  foreach ($find as $item) {
    // Render the node.
    $book = calibre_book_load($item->sid);
    $build = calibre_book_build_content($book, 'search_result');
    unset($build['#theme']);
    $html = drupal_render($build);

    $results[] = array(
      'link' => "/library/book/{$book['id']}",
      'type' => 'Book',
      'title' => $book['title'],
      'user' => '',
      'date' => $book['added']->getTimestamp(),
      'score' => $item->calculated_score,
      'snippet' => search_excerpt($keys, $html),
      'language' => 'en',
    );
  }

  return $results;
}

function calibre_library_cron() {
  $now = time();
  $last_indexed = (int) variable_get('calibre_library_last_cron', 0);

  $bookIDs = array();
  db_set_active('calibre');
  foreach (db_query("SELECT id FROM {books} WHERE CAST(strftime('%s', last_modified) AS INTEGER) >= :last_indexed", array(':last_indexed' => $last_indexed), array('fetch' => PDO::FETCH_ASSOC)) as $result) {
    $bookIDs[] = $result['id'];
  }
  db_set_active();

  if (count($bookIDs)) {
    db_query("UPDATE {search_dataset} SET reindex = 1 WHERE type = 'calibre_book' AND sid IN(:ids)", array(':ids' => $bookIDs));
  }

  variable_set('calibre_library_last_cron', $now);
}
