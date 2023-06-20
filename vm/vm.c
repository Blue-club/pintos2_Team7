/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/mmu.h" // mmu 추가.

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
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
enum vm_type
page_get_type (struct page *page) {
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
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		// 1) 페이지를 생성하고,
		struct page *p = (struct page *)malloc(sizeof(struct page));

		// 2) type에 따라 초기화 함수를 가져와서
		bool (*page_initializer)(struct page *, enum vm_type, void *);

		switch(VM_TYPE(type))
		{
			case VM_ANON:
				page_initializer = anon_initializer;
				break;
			case VM_FILE:
				page_initializer = file_backed_initializer;
				break;
		}

		// 3) "unint" 타입의 페이지로 초기화한다.
		uninit_new(p, upage, init, type, aux, page_initializer);

		// 필드 수정은 unit_new를 호출한 이후에 해야한다.
		p->writable = writable;

		// 4) 생성한 페이지를 SPT에 추가한다.
		return spt_insert_page(spt,p);
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = NULL;
	page = (struct page *)malloc(sizeof(struct page));		// page 의 크기만큼 동적할당해서 공간을 만들어줌.
	
	struct hash_elem *e;					// hash_elem 지정.
	
	page -> va = pg_round_down(va);			// 아래 offset 부분을 FFF으로 만들고 내림한 부분을 va으로 지정.
	e = hash_find(&spt->spt_hash, &page->hash_elem);		// hash 테이블에서 hash_elem의 값을 찾으면
	free(page);								// page는 이제 필요 없음.

	return e != NULL ? hash_entry(e, struct page, hash_elem) : NULL;	// page가 존재한다면 hash에서 
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
			// hash_elem을 넣으려고 했는데, 같은 게 있었으면 false 없었으면 true
			return hash_insert(&spt->spt_hash, &page->hash_elem) == NULL ? true : false;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */
	void *kva = palloc_get_page(PAL_USER); 	// 유저 영역에 대해 물리메모리 페이지를 할당하고 그의 주소를 kva에 넣어준다.

	if (kva == NULL)
		PANIC("todo");
	
	frame = malloc(sizeof(struct frame));	// 핀토스의 커널 영역에서 frame만큼 할당함 (메모리를 단순화 유연성을 제공)
	frame->kva = kva; 						// frame의 kva 맴버에 할당된 유저 영역의 메모리(kva)를 연결한다.
	frame->page = NULL;						// page를 NULL로 초기화 해줘야함.

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	if (addr == NULL)			// 주소가 NULL이면 false
		return false;

	if (is_kernel_vaddr(addr))	// 커널 주소라면 false
		return false;

	if (not_present) // 접근한 메모리의 physical page가 존재하지 않은 경우
	{
		page = spt_find_page(spt, addr);		
		if (page == NULL)
			return false;
		if (write == 1 && page->writable == 0) // write 불가능한 페이지에 write 요청한 경우
			return false;
		return vm_do_claim_page(page);
	}
	return false;
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
	/* 현재 스레드의 spt에서 va에 해당하는 값 찾기. */
	page = spt_find_page(&thread_current()->spt, va);
	if (page == NULL)
		return false;
	/* vm_do_claim_page */
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	/* user pool에서 새로운 physical page - frame이라 불림.(현재 frame->kva는 커널 영역에서 만든 page를 가리키고 있다.)를 가져온다. */
	struct frame *frame = vm_get_frame ();

	/* Set links 페이지와 프레임 간의 연결을 설정하는 작업 */
	frame->page = page;
	page->frame = frame;
	
	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	struct thread *current = thread_current ();
	// bool writable = is_writable(current->pml4); // 오른쪽에서 2번째 비트 ex) 쓰기가능(0x00010)를 확인해서 쓰기 가능한지 확인한다.
	// 여기서 page->writable로 바꿔줬더니 해결! 현재의 pml4가 쓰기 가능인지 확인하는 것이 아니라 page에 저장한 
	/* PML4에 새로운 매핑을 추가하는 역할을 합니다. */
	pml4_set_page(current->pml4, page->va, frame->kva, page->writable);

	/* 스왑 인 */
	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	/* hash 초기화하기. page_hash, page_less 함수를 넣어 초기화 */
	hash_init(&spt->spt_hash, page_hash, page_less, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) 
{
	// TODO: 보조 페이지 테이블을 src에서 dst로 복사합니다.
	// TODO: src의 각 페이지를 순회하고 dst에 해당 entry의 사본을 만듭니다.
	// TODO: uninit page를 할당하고 그것을 즉시 claim해야 합니다.
	struct hash_iterator i;
	hash_first(&i, &src->spt_hash);
	while(hash_next(&i))
	{
		// src_page 정보
		struct page *src_page = hash_entry(hash_cur(&i), struct page, hash_elem);
		enum vm_type type = src_page->operations->type;
		void *upage = src_page->va;
		bool writable = src_page->writable;

		if (type == VM_UNINIT)
		{
			vm_initializer *init = src_page->uninit.init;
			void *aux = src_page->uninit.aux;
			vm_alloc_page_with_initializer(VM_ANON, upage, writable, init, aux);
			continue;
		}

		/* 2) type이 uninit이 아니면 */
		if(!vm_alloc_page_with_initializer(type, upage, writable, NULL, NULL))
			return false;

		if (!vm_claim_page(upage))
			return false;
		
		// 매핑된 프레임에 내용 로딩?
		struct page *dst_page = spt_find_page(dst, upage);
		memcpy(dst_page->frame->kva, src_page->frame->kva, PGSIZE);
	}
	return true;
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	hash_clear(&spt->spt_hash, hash_page_destroy);
}

unsigned
page_hash(const struct hash_elem *p_, void *aux UNUSED)
{
	const struct page *p = hash_entry(p_, struct page, hash_elem);
	return hash_bytes(&p->va, sizeof(p->va));
}

bool
page_less(const struct hash_elem *a_,
	const struct hash_elem *b_, void *aux UNUSED)
{
	const struct page *a = hash_entry(a_, struct page, hash_elem);
	const struct page *b = hash_entry(b_, struct page, hash_elem);

	return a->va < b->va;
}

void hash_page_destroy(struct hash_elem *e, void *aux)
{
	struct page *page = hash_entry(e, struct page, hash_elem);
	destroy(page);
	free(page);
}