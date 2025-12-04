INSERT INTO nexus_stores.cashier_shifts (checkout_id, employee_id, store_id, start_timestamp, end_timestamp)
VALUES 
(
'68cdd3ec'
NULL,
78,
'2059-12-01 13:00:00',
'2059-12-01 15:00:00'
);

INSERT INTO nexus_stores.sales (product_id, checkout_id, store_id, sale_timestamp)
VALUES 
(
(SELECT id FROM nexus_stores.products WHERE name = 'Golden Apple'),
'68cdd3ec',
78,
'2059-12-01 14:47:07'
);

