/* orckit_quirks.h ...... fix quirks in orckit adsl modems
 *                        mulix <mulix@actcom.co.il>
 *
 * $Id: orckit_quirks.h,v 1.1 2005/03/20 05:06:37 rex Exp $
 */

#ifndef INC_ORCKIT_QUIRKS_H_
#define INC_ORCKIT_QUIRKS_H_

#include "pptp_options.h"
#include "pptp_ctrl.h"
#include "pptp_msg.h"

/* return 0 on success, non zero otherwise */
int
orckit_atur3_build_hook(struct pptp_out_call_rqst* packt);

/* return 0 on success, non zero otherwise */
int
orckit_atur3_set_link_hook(struct pptp_set_link_info* packet,
			   int peer_call_id);

/* return 0 on success, non zero otherwise */
int
orckit_atur3_start_ctrl_conn_hook(struct pptp_start_ctrl_conn* packet);

#endif /* INC_ORCKIT_QUIRKS_H_ */
