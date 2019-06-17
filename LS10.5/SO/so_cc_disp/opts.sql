delete from opts 
	where 	opts_access_code = "*" and 
			opts_prog_name = "so_cc_disp";
load from "opts.unl" insert into opts ;
