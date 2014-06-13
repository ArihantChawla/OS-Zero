#ifndef __VT_TEXTBUF_H__
#define __VT_TEXTBUF_H__

void vtfreetextbuf(struct vttextbuf *buf);
long vtinittextbuf(struct vttextbuf *buf, long nrow, long ncol);

#endif /* __VT_TEXTBUF_H__ */

