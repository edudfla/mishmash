#!/usr/bin/env fish
# vim: tabstop=3 shiftwidth=3 softtabstop=3 expandtab cinkeys-=0#
#-------------------------------------------------------------------------------
function main_0 -a work_dir csv_data
   sqlite3 ":memory:" \
      ".import --csv '$csv_data' t" \
      ".mode line" \
      "select * from t limit 1;" \
      ".mode csv" \
      ".headers on" \
      "select count(*) as n, sum(replace(replace(Bedrag, '.', ''), ',', '.')) as v, \"Naam tegenpartij\" as e from t where Datum between '2026-10-01' and '2026-10-31' group by e order by v, n, e;" \
      "with
         a as (select
            replace(replace(Bedrag, '.', ''), ',', '.') as v,
            \"Naam tegenpartij\" as e
         from t
         where
            Datum between '2026-10-01' and '2026-10-31')
      select sum(v)
      from a
      where
         e like 'AH %' or
         e like 'Jumbo %' or
            e like 'ALDI %';"  \
      "with a as (select
         Datum as d,
         replace(replace(Bedrag, '.', ''), ',', '.') as v,
         \"Naam tegenpartij\" as e
      from t)
      select
         substring(d, 1, 7) as m,
         sum(v) as v
      from a
      where
         e like 'AH %' or
         e like 'Jumbo %' or
         e like 'ALDI %'
      group by m;" 2>&1 | less -INRS
end
#-------------------------------------------------------------------------------
function main_1 -a work_dir csv_data
   #
   # An expense which happens in 75% of the months and with a frequency of 0 to
   # two times per month is considered a fixed expense.
   # There are anomalies such as the gym which is charged every 4 weeks and
   # once for every person enrolled (STICHTING DERDENGELDEN BUCKAROO).
   # There is also the "Geldmaat" (e.g. "Geldmaat Godsweerde 74") which is
   # money withdraw from ATM.
   # Maybe these things should be added as exceptions instead of automatically.
   # After adding a longer time span, many fixed expenses disappeared.
   # Improve the logic to include consecutive months instead of all months.
   #
   # Much simpler:
   #   Code = 'ga' is ATM money withdraw
   #   Code = 'ei' is "incasso" (fixed expense)
   #
   set -l csv_data_corrected $work_dir/csv_data_corrected.csv
   sed -E \
      -e 's/"(Van het Land|De Tuin)"/"&"/' \
      -e 's/(LISZT F.REPU:LO|HELLOPAY KERESKEDO)"/&"/' \
      $csv_data > $csv_data_corrected
   sqlite3 ":memory:" \
      ".import --csv '$csv_data_corrected' t_t" \
      "alter table t_t add column m;" \
      "alter table t_t add column e;" \
      "alter table t_t add column v real;" \
      "alter table t_t add column yyyy int;" \
      "alter table t_t add column mm int;" \
      "update t_t set m = substring(Datum, 1, 7);" \
      "update t_t set e = \"Naam tegenpartij\";" \
      "update t_t set v = replace(replace(Bedrag, '.', ''), ',', '.');" \
      "update t_t set yyyy = substring(Datum, 1, 4);" \
      "update t_t set mm = substring(Datum, 5, 2);" \
      "create table expenses_f as select m, sum(v) as v from t_t where Code = 'ei' or e = 'Wise' or e = 'E. M. Bos' group by m;" \
      "create temp table expenses_s as select m, sum(v) as v from t_t
      where
         e like 'AH %' or
         e like 'Jumbo %' or
         e like 'ALDI %' or
         e like 'ALBERT HEIJN %' or
         e like 'Albert Heij %' or
         e like 'Ekoplaza %' or
         e like 'Heezer Geitenkaas %' or
         e like '\"Van het Land\" %' or
         e like 'De Blije Big %'
      group by m;" \
      "create temp table expenses_g as select m, sum(v) as v from t_t where Code = 'ga' group by m;" \
      "create temp table expenses as select m, sum(v) as v from t_t where v < 0 group by m;" \
      "create temp table earnings as select m, sum(v) as v from t_t where v > 0 group by m;" \
      "create temp table months as select m from t_t group by m order by m;" \
      ".mode table" \
      ".width -9 -9 -9 -9 -9 -9 -9" \
      "select
         months.m as month,
         format('%.2f', expenses_f.v ) as fixed,
         format('%.2f', expenses_s.v ) as grocery,
         format('%.2f', coalesce(expenses_g.v, 0)) as cash,
         format('%.2f', expenses.v - expenses_f.v - expenses_s.v - coalesce(expenses_g.v, 0)) as other,
         format('%.2f', earnings.v ) as earnings,
         format('%.2f', earnings.v + expenses.v) as saldo
      from
         months left join
         expenses_f on expenses_f.m = months.m left join
         expenses_s on expenses_s.m = months.m left join
         expenses_g on expenses_g.m = months.m left join
         expenses on expenses.m = months.m left join
         earnings on earnings.m = months.m
      ;"

      #
      # After some digging, it was seen that when field "Code" is equal to
      # "ei" the entry is for a fixed expense.
      #
      # "create temp table t_fixed as with
      #    t_0 as (select count(*) as n, m, e from t_t where
      #       v < 0 and
      #       e not like 'Geldmaat %' and
      #       e <> 'Stijl ROERMOND' and
      #       e <> 'Holland Barrett'
      #    group by m, e),
      #    t_a as (select min(m) as min_m, max(m) as max_m, count(*) as m_count, sum(n) as n, e from t_0 group by e),
      #    t_b as (select
      #       12 * (substring(max_m, 1, 4) - substring(min_m, 1, 4)) + substring(max_m, 6, 2) - substring(min_m, 6, 2) + 1 as m_span,
      #       m_count, n, e
      #    from t_a)
      # select * from t_b
      # where
      #    e = 'STICHTING DERDENGELDEN BUCKAROO' -- gym exception
      #    or e = 'Fit for Free Beheer B.V.' -- gym exception
      #    or e = 'FIT FOR FREE BEHEER BV' -- gym exception
      #    or e = 'Fit For Free' -- gym exception
      #    or e = 'Wise' -- alimony with interruptions
      #    or (
      #       m_count > 6
      #       and 1.0 * n / m_count between 0.75 and 1.33
      #       and 1.0 * m_count / m_span between 0.75 and 1.33
      #    )
      # ;" \
      # ".headers on" \
      # "select * from t_fixed;" \
      # "create temp table expenses_f as with
      #    t_a as (select * from t_t left jon t_fixed on t_t.e = t_fixed.e)
      # select t_a.m, sum(v) as v, group_concat(e, ', ') as e from t_a where m_span is not null and v < 0 group by t_a.m;" \

      # First version of "t_fixed"
      #
      # "
      # --
      # -- t_f is the frequency per month and establishment.
      # --
      # create temp table t_f as
      # select
      #    m,
      #    e,
      #    count(*) as n -- number of occurrences per month and establishment
      # from t_t
      # group by m, e;" \
      # "  -- select * from t_f;" \
      # "create table t_fixed as with
      #    t_a as (select count(*) as n_m, sum(n) as n_n, e from t_f where n <= 2 group by e),
      #    t_b as (select m from t_f group by m),
      #    t_c as (select count(*) as n from t_b),
      #    t_d as (select *, 1.0 * n_m / n_n as coef_1, 1.0 * n_n / n as coef_2 from t_a left join t_c)
      # select * from t_d where 1.0 * n_n / n_m between 0.5 and 2 and 1.0 * n_n / n >= 0.75 and not e like 'Geldmaat %'
      # -- select count(*) from t_a union all
      # -- select count(*) from t_b union all
      # -- select count(*) from t_c
      # -- select * from t_b
      # ;" \

end
#-------------------------------------------------------------------------------
function main_2 -a work_dir csv_data
   set -l csv_data_corrected $work_dir/csv_data_corrected.csv
   sed -E \
      -e 's/"(Van het Land|De Tuin)"/"&"/' \
      -e 's/(LISZT F.REPU:LO|HELLOPAY KERESKEDO)"/&"/' \
      $csv_data > $csv_data_corrected
   set db $work_dir/db.sqlite3
   sqlite3 $db \
      ".import --csv '$csv_data_corrected' t"
   #
   # Presumed fixed expenses
   #
   sqlite3 $db \
      "create temp table t_fixed as with
         t_0 as (select count(*) as n, substring(Datum, 1, 7) as m, \"Naam tegenpartij\" as e from t where
            cast(replace(replace(Bedrag, '.', ''), ',', '.') as real) < 0 and
            Code <> 'ei'
         group by m, e),
         t_a as (select min(m) as min_m, max(m) as max_m, count(*) as m_count, sum(n) as n, e from t_0 group by e),
         t_b as (select
            12 * (substring(max_m, 1, 4) - substring(min_m, 1, 4)) + substring(max_m, 6, 2) - substring(min_m, 6, 2) + 1 as m_span,
            m_count, n, e
         from t_a)
      select * from t_b
      where
         m_count > 6
         and 1.0 * n / m_count between 0.75 and 1.33
         and 1.0 * m_count / m_span between 0.75 and 1.33
      ;" \
      ".headers on" \
      "select * from t_fixed;" \

   return

   # sqlite3 $db \
   #    ".mode line" \
   #    "select * from t limit 1;" |
   # rg --no-unicode -e '^\s*(.*) = .*' -r '$1'
   set -l fields (sqlite3 $db \
      ".mode line" \
      "select * from t limit 1;" |
      rg --no-unicode -e '^\s*(.*) = .*' -r '$1')
   for field in $fields
      echo (sqlite3 $db \
         "with a as (select count(*) from t group by \"$field\")
      select count(*) from a;") $field
   end |
   sort -n |
   while read n f
      if test 2 -le $n -a 50 -ge $n
         echo $n $f
         printf '\t%s\n' (sqlite3 $db "select count(*) as n, \"$f\" from t group by \"$f\";")
      end
   end


   sqlite3 $db \
      ".mode table" \
      "select count(*) as n, Code from t where \"Naam tegenpartij\" like 'Geldmaat %' group by Code;"
   sqlite3 $db \
      ".mode table" \
      "select count(*) as n, \"Naam tegenpartij\" from t where Code = 'ga' group by \"Naam tegenpartij\";"
   #sqlite3 $db \
   #   ".mode table" \
   #   "select count(*) as n, Code, \"Naam tegenpartij\" from t group by Code, \"Naam tegenpartij\" order by Code, n, \"Naam tegenpartij\";"
   sqlite3 $db \
      ".mode table" \
      "select count(*) as n, Code, \"Naam tegenpartij\" from t where \"Incassant ID\" <> '' group by \"Naam tegenpartij\", \"Incassant ID\";"
   sqlite3 $db \
      ".mode table" \
      "select count(*) as n, \"Naam tegenpartij\", \"Incassant ID\" from t where Code = 'ei' group by \"Naam tegenpartij\", \"Incassant ID\";"
end
#-------------------------------------------------------------------------------
function main
   set -l work_dir (mktemp --directory --tmpdir)
   trap "rm -f -r $work_dir" EXIT INT QUIT TERM
   switch "$argv[1]"
      case "main_0"
         main_0 $work_dir $argv[2..]
      case "main_1"
         main_1 $work_dir $argv[2..]
      case "main_2"
         main_2 $work_dir $argv[2..]
      case "*"
         main_1 $work_dir $argv
   end
end
#-------------------------------------------------------------------------------
main $argv
#-------------------------------------------------------------------------------
