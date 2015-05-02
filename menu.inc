<?php

function calibre_library_menu(){
  return array(

    'library' => array(
      'type'              => MENU_NORMAL_ITEM,
      'title'             => 'Library',
      'description'       => 'Catalog Home',
      'access callback'   => 'user_access',
      'access arguments'  => array('calibre_library_view_catalog'),
      'page callback'     => 'calibre_library_catalog_view',
     ),

     'library/new' => array(
       'type'              => MENU_DEFAULT_LOCAL_TASK,
       'title'             => 'New',
       'description'       => 'Latest additions to the Catalog',
       'access callback'   => 'user_access',
       'access arguments'  => array('calibre_library_view_catalog'),
       'page callback'     => 'calibre_library_catalog_view',
       'weight'            => -10,
      ),

    'library/book/%' => array(
      'type'              => MENU_CALLBACK,
      'title'             => 'Book',
      'description'       => 'Book Information',
      'access callback'   => 'user_access',
      'access arguments'  => array('calibre_library_view_catalog'),
      'page callback'     => 'calibre_library_book_view',
      'page arguments'    => array(2),
     ),

     'library/authors' => array(
       'type'              => MENU_LOCAL_TASK,
       'title'             => 'Authors',
       'description'       => 'List of authors in the collection',
       'access callback'   => 'user_access',
       'access arguments'  => array('calibre_library_view_catalog'),
       'page callback'     => 'calibre_library_authors_list',
     ),

     'library/author/%' => array(
      'type'              => MENU_CALLBACK,
      'title'             => 'Author',
      'description'       => 'Books by Author',
      'access callback'   => 'user_access',
      'access arguments'  => array('calibre_library_view_catalog'),
      'page callback'     => 'calibre_library_author_view',
      'page arguments'    => array(2),
     ),

     'library/tags' => array(
       'type'              => MENU_LOCAL_TASK,
       'title'             => 'Tags',
       'description'       => 'List of tags in the collection',
       'access callback'   => 'user_access',
       'access arguments'  => array('calibre_library_view_catalog'),
       'page callback'     => 'calibre_library_tags_list',
     ),

     'library/tag/%' => array(
      'type'              => MENU_CALLBACK,
      'title'             => 'Tag',
      'description'       => 'Books by tag',
      'access callback'   => 'user_access',
      'access arguments'  => array('calibre_library_view_catalog'),
      'page callback'     => 'calibre_library_tag_view',
      'page arguments'    => array(2),
     ),

     'library/series' => array(
       'type'              => MENU_LOCAL_TASK,
       'title'             => 'Series',
       'description'       => 'List of series in the collection',
       'access callback'   => 'user_access',
       'access arguments'  => array('calibre_library_view_catalog'),
       'page callback'     => 'calibre_library_series_list',
     ),

    'library/series/%' => array(
      'type'              => MENU_CALLBACK,
      'title'             => 'Series',
      'description'       => 'Books by series',
      'access callback'   => 'user_access',
      'access arguments'  => array('calibre_library_view_catalog'),
      'page callback'     => 'calibre_library_series_view',
      'page arguments'    => array(2),
     ),
  );
}
