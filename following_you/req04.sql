SELECT 'metro' AS place_type, COUNT(DISTINCT person_id) AS entries, station_id AS place_id
FROM transport.metro_usage_logs
GROUP BY station_id
UNION
SELECT 'shop' AS place_type, COUNT(DISTINCT person_id) AS entries, shop_id AS place_id
FROM public.shop_entrance_logs
GROUP BY shop_id
ORDER BY entries DESC, place_id ASC, place_type DESC;
