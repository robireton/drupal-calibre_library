<?php

function name_list_string($names, $conj = 'or') {
  $tail = array_pop($names);
  $head = implode(', ', $names);
  return sprintf('%s%s%s', $head, $head ? " $conj " : '', $tail);
}
