#pragma once

void setup_pmm();
void *allocate_page();
void free_page(void *page);
