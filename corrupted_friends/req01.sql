CREATE TYPE public.prefix AS ENUM ('MRS', 'MS', 'MR', 'DR');

CREATE TABLE dtf.madelines_contacts (
    id INTEGER PRIMARY KEY,
    title public.prefix,
    first_name TEXT,
    last_name TEXT,
    phone TEXT,
    email TEXT,
    favorite BOOLEAN,
    created_at TIMESTAMP
);

