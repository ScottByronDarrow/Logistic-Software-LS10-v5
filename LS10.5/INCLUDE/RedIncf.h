/*===============================================
| Reduce incf_fifo_qty's for incf records    	|
| to allow stock transfers between warehouses	|
| to work correctly.							|
===============================================*/
void
red_incf (
 long	hhwhHash,
 float	closingStock,
 int	_fifo)
{
	int	rc;
	float	qty = closingStock;
	
	if (qty < 0.00)
		qty = 0.00;

	rc = FindFifo (hhwhHash,0L,!_fifo,"u");
	
	while (!rc && hhwhHash == incf_rec.hhwh_hash)
	{
		/*-----------------------
		| need to reduce ff_qty	|
		-----------------------*/
		if (qty < incf_rec.fifo_qty)
		{
			incf_rec.fifo_qty = qty;
			qty = 0.00;
			rc = abc_update ("incf",&incf_rec);
			if (rc)
				file_err (cc, "incf", "DBUPDATE");
		}
		else
			qty  -= incf_rec.fifo_qty;

		abc_unlock ("incf");

		rc = FindFifo (0L,0L,!_fifo,"u");
	}
	abc_unlock ("incf");
}
