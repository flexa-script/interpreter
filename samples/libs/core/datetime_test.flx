using cp.core.datetime;

as namespace cp;

// defaults UTC
var dt = create_date_time();

println("generated date: ", dt);

var dtnl = create_date_time(1900, 1, 1, 0, 0, 0);

println("null date: ", dtnl);

println("diff: ", diff_date_time(dt, dtnl));

println("format_date_time: ", format_date_time(dt, "%Y-%m-%d %H:%M:%S"));
println("format_local_date_time: ", format_local_date_time(dt, "%Y-%m-%d %H:%M:%S"));
println("ascii_date_time: ", ascii_date_time(dt));
println("ascii_local_date_time: ", ascii_local_date_time(dt));
