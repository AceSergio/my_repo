CREATE TABLE dtf.shifts_and_sales_78 AS
SELECT
nexus_stores.sales.id AS sales_id,
nexus_stores.sales.product_id,
nexus_stores.sales.checkout_id,
nexus_stores.sales.sale_timestamp,
nexus_stores.cashier_shifts.id AS shift_id,
nexus_stores.cashier_shifts.employee_id,
nexus_stores.cashier_shifts.start_timestamp AS shift_start_timestamp,
nexus_stores.cashier_shifts.end_timestamp AS shift_end_timestamp
FROM nexus_stores.sales S
JOIN nexus_stores.cashier_shifts
ON nexus_stores.sales.checkout_id = nexus_stores.cashier_shifts.checkout_id
AND nexus_stores.sales.store_id = nexus_stores.cashier_shifts.store_id
WHERE nexus_stores.sales.store_id = 78
AND nexus_stores.sales.sale_timestamp
BETWEEN nexus_stores.cashier_shifts.start_timestamp AND nexus_stores.cashier_shifts.end_timestamp;

