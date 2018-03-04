SELECT
 CONCAT(z.expected, IF(z.got-1>z.expected, CONCAT(' through ',z.got-1), '')) AS freecreatureguids
FROM (
 SELECT
  @rownum:=@rownum+1 AS expected,
  IF(@rownum=guid, 0, @rownum:=guid) AS got
 FROM
  (SELECT @rownum:=0) AS a
  JOIN creature
  ORDER BY guid
 ) AS z
WHERE z.got!=0;

SELECT
 CONCAT(z.expected, IF(z.got-1>z.expected, CONCAT(' through ',z.got-1), '')) AS freegameobjectguids
FROM (
 SELECT
  @rownum:=@rownum+1 AS expected,
  IF(@rownum=guid, 0, @rownum:=guid) AS got
 FROM
  (SELECT @rownum:=0) AS a
  JOIN gameobject
  ORDER BY guid
 ) AS z
WHERE z.got!=0;
