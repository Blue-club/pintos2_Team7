/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "threads/mmu.h"
#include "threads/thread.h"


/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init (void) 
{
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type page_get_type (struct page *page) 
{
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable, vm_initializer *init, void *aux) 
{

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page * spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) 
{
	struct page *page = NULL;
	/* TODO: Fill this function. */
	/*pg_round_down페이지 경계에 맞춰 반내림*/
	/*ex) 0x00001234를 pg_round_down 하면 0x00001000으로, 페이지 크기나 정렬방식은 시스템의 페이지에 맞게 동작  */
    struct page dummy_page; dummy_page.va = pg_round_down(va); // dummy for hashing
    struct hash_elem *e;

	/* hash_elem 구조체를 e로 받아온다 */
    e = hash_find(&spt->spt_hash, &dummy_page.hash_elem);

	if(e == NULL)
		return NULL;

    return page = hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
/*깃북 : 주어진 supplemental page table에 struct page를 삽입하시오
이 함수는 가상 주소가 주어진 supplemental page table에 존재하지 않는지 확인해야 합니다*/
bool spt_insert_page (struct supplemental_page_table *spt UNUSED, struct page *page UNUSED) 
{
	int succ = false;
	/* TODO: Fill this function. */
	// spt에 존재하지 않는지 확인?
	struct hash_elem *e = hash_find(&spt->spt_hash, &page->hash_elem);
	if(e != NULL)
	{
		return succ;
	}
	//page가 spt에 없으면 삽입
	hash_insert(&spt->spt_hash, &page->hash_elem);
	return succ = true;
}

bool delete_page (struct hash *pages, struct page *p)
{
	if (hash_delete(pages, &p->hash_elem))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void spt_remove_page (struct supplemental_page_table *spt, struct page *page) 
{
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame * vm_get_victim (void) 
{
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */
	victim = list_entry(list_pop_front(&frame_table),struct frame, elem);
	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
/*제거(evict)를 swap_out으로 하는건가?
그럼 제거된게 아니니깐 다시 swap_in으로 불러올수있을지도?*/
static struct frame * vm_evict_frame (void) 
{
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */
	if(victim->page != NULL)
	{
		swap_out(victim->page);
	}
	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame * vm_get_frame (void) 
{
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
	/*PAL_USER를 사용 하는 이유: 커널 풀 대신 사용자 풀에서 메모리를 할당하게 하기 위함.
	사용자 풀의 페이지가 부족하면 사용자 프로그램의 페이지가 부족해지지만, 커널 풀의 페이지가 부족하면
	많은 커널 함수들이 메모리를 확보하는데 문제가 생길수 잇음*/
	
	void *kva = palloc_get_page(PAL_USER);
	if (kva == NULL) // page 할당 실패
	{
		frame = vm_evict_frame();
	}
	else
	{
		frame = malloc(sizeof(struct frame));
		frame->kva = kva;
	}

	ASSERT (frame != NULL);
	return frame;
}

/* Growing the stack. */
static void vm_stack_growth (void *addr UNUSED) 
{

}

/* Handle the fault on write_protected page */
static bool vm_handle_wp (struct page *page UNUSED) 
{

}

/* Return true on success */
bool vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED, bool user UNUSED, bool write UNUSED, bool not_present UNUSED) 
{
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page (struct page *page) 
{
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
//페이지를 va에 할당
bool vm_claim_page (void *va UNUSED) 
{
	struct page *page = NULL;
	/* TODO: Fill this function */
	page = spt_find_page(&thread_current()->spt, va);
	if (page == NULL)
		return false;

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
/*claim은 physical frame인 페이지를 할당?*/
/*vm_get_frame을 호출,매핑*/
static bool vm_do_claim_page (struct page *page) 
{
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	// 가상 주소와 물리 주소를 매핑
	struct thread *cur = thread_current();
	bool writable = page->writable;
	pml4_set_page(cur->pml4, page->va, frame->kva, page->writable);

	bool res = swap_in (page, frame->kva);
	return res;
}

/* Initialize new supplemental page table */
/*supplemental_page_table 구현 필요*/
void supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) 
{
	/*깃북 "Initializes the supplemental page table." */
	/*hash table을 사용하는 이유 : pintos에서 hash 지원해줌*/
	/*page_hash, page_less는 hash_init에서 포인터로 받기 때문에 함수 실행이 아니다*/
	hash_init(&spt->spt_hash, page_hash, page_less, NULL);

}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED, struct supplemental_page_table *src UNUSED) 
{

}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) 
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}

/* Returns a hash value for page p. */
// 해시테이블 초기화 할때 해시 값을 구해주는 함수의 포인터
unsigned page_hash(const struct hash_elem *p_, void *aux UNUSED)
{
	/*hash_elem은 prev와 next 값이 들어있으니 걔넬 포함하고있는 우리가 사용
	하려고 하는 hash값을 찾는? 건가?*/
	//hash_elem이 속한 페이지 주소알아냄
	const struct page *p = hash_entry(p_, struct page, hash_elem);

	/*그럼 hash_bytes가 hash func 이 되는건가?*/
	//p의 해시값 반환
	return hash_bytes(&p->va, sizeof p->va);
}

/* Returns true if page a precedes page b. */
/*페이지의 순서 비교*/
/*효율때문에?*/
bool page_less(const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED)
{
	const struct page *a = hash_entry(a_, struct page, hash_elem);
	const struct page *b = hash_entry(b_, struct page, hash_elem);

	return a->va < b->va;
}