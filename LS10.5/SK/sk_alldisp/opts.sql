delete from opts 
	where 	opts_access_code = "*" and 
			opts_prog_name = "sk_alldisp";
load from "opts.unl" insert into opts ;
