delete from opts 
	where 	opts_access_code = "*" and 
			opts_prog_name = "po_ss_disp";
load from "opts.unl" insert into opts ;
