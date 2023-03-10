/* i810_bufs.c -- IOCTLs to manage buffers -*- linux-c -*-
 * Created: Thu Jan 6 01:47:26 2000 by jhartmann@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * Authors: Rickard E. (Rik) Faith <faith@valinux.com>
 *	    Jeff Hartmann <jhartmann@valinux.com>
 * 
 */

#define __NO_VERSION__
#include "drmP.h"
#include "i810_drv.h"
#include "linux/un.h"

int i810_addbufs_agp(struct inode *inode, struct file *filp, unsigned int cmd,
		    unsigned long arg)
{
	drm_file_t *priv = filp->private_data;
	drm_device_t *dev = priv->dev;
	drm_device_dma_t *dma = dev->dma;
	drm_buf_desc_t request;
	drm_buf_entry_t *entry;
	drm_buf_t *buf;
	unsigned long offset;
	unsigned long agp_offset;
	int count;
	int order;
	int size;
	int alignment;
	int page_order;
	int total;
	int byte_count;
	int i;

	if (!dma) return -EINVAL;

	if (copy_from_user(&request,
			   (drm_buf_desc_t *)arg,
			   sizeof(request)))
		return -EFAULT;

	count = request.count;
	order = drm_order(request.size);
	size	= 1 << order;
	agp_offset = request.agp_start;
	alignment  = (request.flags & _DRM_PAGE_ALIGN) ? PAGE_ALIGN(size) :size;
	page_order = order - PAGE_SHIFT > 0 ? order - PAGE_SHIFT : 0;
	total = PAGE_SIZE << page_order;
	byte_count = 0;
   
	if (order < DRM_MIN_ORDER || order > DRM_MAX_ORDER) return -EINVAL;
	if (dev->queue_count) return -EBUSY; /* Not while in use */
	spin_lock(&dev->count_lock);
	if (dev->buf_use) {
		spin_unlock(&dev->count_lock);
		return -EBUSY;
	}
	atomic_inc(&dev->buf_alloc);
	spin_unlock(&dev->count_lock);
   
	down(&dev->struct_sem);
	entry = &dma->bufs[order];
	if (entry->buf_count) {
		up(&dev->struct_sem);
		atomic_dec(&dev->buf_alloc);
		return -ENOMEM; /* May only call once for each order */
	}
   
	entry->buflist = drm_alloc(count * sizeof(*entry->buflist),
				   DRM_MEM_BUFS);
	if (!entry->buflist) {
		up(&dev->struct_sem);
		atomic_dec(&dev->buf_alloc);
		return -ENOMEM;
	}
	memset(entry->buflist, 0, count * sizeof(*entry->buflist));
   
	entry->buf_size   = size;
	entry->page_order = page_order;
	offset = 0;
   
	while(entry->buf_count < count) {
		buf = &entry->buflist[entry->buf_count];
		buf->idx = dma->buf_count + entry->buf_count;
		buf->total = alignment;
		buf->order = order;
		buf->used = 0;
		buf->offset = offset;
		buf->bus_address = dev->agp->base + agp_offset + offset;
		buf->address = (void *)(agp_offset + offset + dev->agp->base);
		buf->next = NULL;
		buf->waiting = 0;
		buf->pending = 0;
		init_waitqueue_head(&buf->dma_wait);
		buf->pid = 0;

		buf->dev_private = drm_alloc(sizeof(drm_i810_buf_priv_t), 
					     DRM_MEM_BUFS);
		buf->dev_priv_size = sizeof(drm_i810_buf_priv_t);
	   	memset(buf->dev_private, 0, sizeof(drm_i810_buf_priv_t));

#if DRM_DMA_HISTOGRAM
		buf->time_queued = 0;
		buf->time_dispatched = 0;
		buf->time_completed = 0;
		buf->time_freed = 0;
#endif
		offset = offset + alignment;
		entry->buf_count++;
		byte_count += PAGE_SIZE << page_order;
      
		DRM_DEBUG("buffer %d @ %p\n",
			  entry->buf_count, buf->address);
	}
   
	dma->buflist = drm_realloc(dma->buflist,
				   dma->buf_count * sizeof(*dma->buflist),
				   (dma->buf_count + entry->buf_count)
				   * sizeof(*dma->buflist),
				   DRM_MEM_BUFS);
	for (i = dma->buf_count; i < dma->buf_count + entry->buf_count; i++)
		dma->buflist[i] = &entry->buflist[i - dma->buf_count];
   
	dma->buf_count  += entry->buf_count;
	dma->byte_count += byte_count;
	drm_freelist_create(&entry->freelist, entry->buf_count);
	for (i = 0; i < entry->buf_count; i++) {
		drm_freelist_put(dev, &entry->freelist, &entry->buflist[i]);
	}
   
	up(&dev->struct_sem);
   
	request.count = entry->buf_count;
	request.size  = size;
   
	if (copy_to_user((drm_buf_desc_t *)arg,
			 &request,
			 sizeof(request)))
		return -EFAULT;
   
	atomic_dec(&dev->buf_alloc);
	dma->flags = _DRM_DMA_USE_AGP;
	return 0;
}

int i810_addbufs(struct inode *inode, struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	drm_buf_desc_t	 request;

	if (copy_from_user(&request,
			   (drm_buf_desc_t *)arg,
			   sizeof(request)))
		return -EFAULT;

	if(request.flags & _DRM_AGP_BUFFER)
		return i810_addbufs_agp(inode, filp, cmd, arg);
	else
		return -EINVAL;
}

int i810_infobufs(struct inode *inode, struct file *filp, unsigned int cmd,
		 unsigned long arg)
{
	drm_file_t	 *priv	 = filp->private_data;
	drm_device_t	 *dev	 = priv->dev;
	drm_device_dma_t *dma	 = dev->dma;
	drm_buf_info_t	 request;
	int		 i;
	int		 count;

	if (!dma) return -EINVAL;

	spin_lock(&dev->count_lock);
	if (atomic_read(&dev->buf_alloc)) {
		spin_unlock(&dev->count_lock);
		return -EBUSY;
	}
	++dev->buf_use;		/* Can't allocate more after this call */
	spin_unlock(&dev->count_lock);

	if (copy_from_user(&request,
			   (drm_buf_info_t *)arg,
			   sizeof(request)))
		return -EFAULT;

	for (i = 0, count = 0; i < DRM_MAX_ORDER+1; i++) {
		if (dma->bufs[i].buf_count) ++count;
	}
	
	DRM_DEBUG("count = %d\n", count);
	
	if (request.count >= count) {
		for (i = 0, count = 0; i < DRM_MAX_ORDER+1; i++) {
			if (dma->bufs[i].buf_count) {
				if (copy_to_user(&request.list[count].count,
						 &dma->bufs[i].buf_count,
						 sizeof(dma->bufs[0]
							.buf_count)) ||
				    copy_to_user(&request.list[count].size,
						 &dma->bufs[i].buf_size,
						 sizeof(dma->bufs[0].buf_size)) ||
				    copy_to_user(&request.list[count].low_mark,
						 &dma->bufs[i]
						 .freelist.low_mark,
						 sizeof(dma->bufs[0]
							.freelist.low_mark)) ||
				    copy_to_user(&request.list[count]
						 .high_mark,
						 &dma->bufs[i]
						 .freelist.high_mark,
						 sizeof(dma->bufs[0]
							.freelist.high_mark)))
					return -EFAULT;

				DRM_DEBUG("%d %d %d %d %d\n",
					  i,
					  dma->bufs[i].buf_count,
					  dma->bufs[i].buf_size,
					  dma->bufs[i].freelist.low_mark,
					  dma->bufs[i].freelist.high_mark);
				++count;
			}
		}
	}
	request.count = count;

	if (copy_to_user((drm_buf_info_t *)arg,
			 &request,
			 sizeof(request)))
		return -EFAULT;
	
	return 0;
}

int i810_markbufs(struct inode *inode, struct file *filp, unsigned int cmd,
		 unsigned long arg)
{
	drm_file_t	 *priv	 = filp->private_data;
	drm_device_t	 *dev	 = priv->dev;
	drm_device_dma_t *dma	 = dev->dma;
	drm_buf_desc_t	 request;
	int		 order;
	drm_buf_entry_t	 *entry;

	if (!dma) return -EINVAL;

	if (copy_from_user(&request,
			   (drm_buf_desc_t *)arg,
			   sizeof(request)))
		return -EFAULT;

	DRM_DEBUG("%d, %d, %d\n",
		  request.size, request.low_mark, request.high_mark);
	order = drm_order(request.size);
	if (order < DRM_MIN_ORDER || order > DRM_MAX_ORDER) return -EINVAL;
	entry = &dma->bufs[order];

	if (request.low_mark < 0 || request.low_mark > entry->buf_count)
		return -EINVAL;
	if (request.high_mark < 0 || request.high_mark > entry->buf_count)
		return -EINVAL;

	entry->freelist.low_mark  = request.low_mark;
	entry->freelist.high_mark = request.high_mark;
	
	return 0;
}

int i810_freebufs(struct inode *inode, struct file *filp, unsigned int cmd,
		 unsigned long arg)
{
	drm_file_t	 *priv	 = filp->private_data;
	drm_device_t	 *dev	 = priv->dev;
	drm_device_dma_t *dma	 = dev->dma;
	drm_buf_free_t	 request;
	int		 i;
	int		 idx;
	drm_buf_t	 *buf;

	if (!dma) return -EINVAL;

	if (copy_from_user(&request,
			   (drm_buf_free_t *)arg,
			   sizeof(request)))
		return -EFAULT;

	DRM_DEBUG("%d\n", request.count);
	for (i = 0; i < request.count; i++) {
		if (copy_from_user(&idx,
				   &request.list[i],
				   sizeof(idx)))
			return -EFAULT;
		if (idx < 0 || idx >= dma->buf_count) {
			DRM_ERROR("Index %d (of %d max)\n",
				  idx, dma->buf_count - 1);
			return -EINVAL;
		}
		buf = dma->buflist[idx];
		if (buf->pid != current->pid) {
			DRM_ERROR("Process %d freeing buffer owned by %d\n",
				  current->pid, buf->pid);
			return -EINVAL;
		}
		drm_free_buffer(dev, buf);
	}
	
	return 0;
}

