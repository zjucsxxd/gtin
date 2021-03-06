--
-- Tests for the GTIN data type.
--
--

--
-- first, define the datatype.  Turn off echoing so that expected file
-- does not depend on contents of gtin.sql.
--
SET client_min_messages = warning;
\set ECHO none
--\set ON_ERROR_ROLBACK 1
--\set ON_ERROR_STOP true
--BEGIN;
\i sql/gtin.sql
RESET client_min_messages;
\set ECHO all

CREATE TEMP TABLE product (
    gtin    gtin UNIQUE PRIMARY KEY,
    name    text
);

-- expect empty GTIN ERROR
insert into product values ('', 'Empty GTIN');
-- expect bad characters error
insert into product values ('400763+0000116', 'Invalid Character');
-- expect long GTIN error.
insert into product values ('1231231231234007630000116', 'Too Long');
-- expect invalid checksum ERROR
insert into product values ('4007630000117', 'Invalid Checksum');
-- expect PK ERROR
insert into product values ('4007630000116', 'PK');
insert into product values ('4007630000116', 'PK');

-- Expect No errors
insert into product values ('0061414000024',  'Arthur Dent');
insert into product values ('0012345-000065', 'Ford Prefect');
insert into product values ('614141 000418',  'Zaphod Beeblebrox');
insert into product values ('10614141000415', 'Trillian');
insert into product values ('10614141450005', 'Slartibartfast');
insert into product values ('10614141777775', 'Zarniwoop');
insert into product values ('123',            'Milliways');

-- Accessor functions
select gtin, isa_gtin(gtin) from product;
-- Expect trues
select isa_gtin('0061414000024'), isa_gtin('00614 1400 0024'),
       isa_gtin('00614-1400-0024');
-- Expect false
select isa_gtin('0061414000025'), isa_gtin(''), isa_gtin('12+3'),
       isa_gtin('1231231231234007630000116');

-- Equality tests
select '0061414000024'::gtin = '61414000024'::gtin AS is_same_gtin;
select '6141-400 0024'::gtin = '61414000024'::gtin AS is_same_gtin;

select name, gtin from product where gtin = '0061414000024';
select name, gtin from product where gtin = '006 1414-000024';
select a.gtin, a2.name from product a JOIN product a2 USING (gtin);
select a.gtin, a2.name from product a JOIN product a2 ON (UPPER(a.gtin)::gtin = a2.gtin);

-- Comparison Operators 
-- Casting is implicit.
select '123'::gtin   <  '130' AS lt;
select '123'::gtin   <= '130' AS le1;
select '123'::gtin   <= '123' AS le2;
select '00123'::gtin <= '130' AS le3;
select '130'::gtin   >  '123' AS gt;
select '130'::gtin   >= '123' AS ge1;
select '130'::gtin   >= '130' AS ge2;
select '00130'::gtin >= '130' AS ge3;

select gtin from product where gtin <= '61414000024';
select gtin from product where gtin >= '0061414000024';

-- LIKE 
-- Implicitly casts values
select name, gtin from product where gtin like '106%'; 
select name, gtin from product where gtin ilike '106%'; 
select name, gtin from product where gtin ~~ '106%';

-- select true where constant like constant works
select true where '0061414000024'::gtin like '614%';
select true where '0061414000024'::gtin ilike '614%';
select true where '0061414000024'::gtin ~~ '614%';

-- Regular Expressions.
-- Implicitly casts values
select name, gtin from product where gtin ~ '^106'; 
select name, gtin from product where gtin ~* '^106'; 

-- select true where constant ~ constant works
select true where '0061414000024'::gtin ~ '^614';
select true where '0061414000024'::gtin ~* '^614';

-- ORDER BY ASC:
select * from product order by gtin;
select * from product order by gtin ASC;

-- ORDER BY DESC:
select * from product order by gtin DESC;

-- FORMATTING
select to_char('123'::gtin, '999')    = '123';
select to_char('123'::gtin, '99999')  = '  123';
select to_char('123'::gtin, '99-9')   = '12-3';
select to_char('123'::gtin, '999-99') = '  1-23';
select to_char('123'::gtin, '000')    = '123';
select to_char('123'::gtin, '00000')  = '00123';
select to_char('123'::gtin, '00-0')   = '12-3';
select to_char('123'::gtin, '000-00') = '001-23';
