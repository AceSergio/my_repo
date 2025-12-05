SELECT
    p.id AS person_id,
    p.first_name AS person_first_name,
    p.last_name AS person_last_name,
    activity.created_at,
    activity.place,
    activity.place_id
FROM (
    SELECT created_at, 'street' AS place, street_id AS place_id, person_id FROM public.street_logs
    UNION ALL
    SELECT created_at, 'metro' AS place, station_id AS place_id, person_id FROM transport.metro_usage_logs
    UNION ALL
    SELECT created_at, 'shop' AS place, shop_id AS place_id, person_id FROM public.shop_entrance_logs
) AS activity
JOIN public.people p ON activity.person_id = p.id
WHERE activity.created_at BETWEEN '2059-12-03 17:00:00' AND '2059-12-03 22:00:00'
ORDER BY activity.created_at ASC, person_id ASC;
