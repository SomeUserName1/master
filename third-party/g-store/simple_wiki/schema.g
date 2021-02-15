CREATE GRAPH wiki_simple(
vid       UINT(4) IS ID,
title     VARCHAR,
wid       UINT(4),
creation  CHAR(20),
cid       UINT(4),
user      VARCHAR,
abstract  VARCHAR) 
FROM "simple_wiki/simple_wiki.g"
;