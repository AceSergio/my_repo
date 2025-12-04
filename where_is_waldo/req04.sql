SELECT
nexus_stores.cashier_shifts.id,
nexus_stores.cashier_shifts.checkout_id,
nexus_stores.cashier_shifts.employee_id,
nexus_stores.cashier_shifts.store_id AS shift_store_id,
nexus_stores.checkouts.store_id AS checkout_store_id,
nexus_stores.cashier_shifts.start_timestamp,
nexus_stores.cashier_shifts.end_timestamp
FROM nexus_stores.cashier_shifts
JOIN nexus_stores.checkouts ON nexus_stores.cashier_shifts.checkout_id = nexus_stores.checkouts.id;


