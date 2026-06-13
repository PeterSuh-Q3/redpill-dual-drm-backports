# amd/amdgpu/ — 5.4 기반 amdgpu OOT 추출 대상

5.4 LTS amdgpu + display(DC) 트리 추출.
주의(§6.3): 5.4 는 ttm_mem_reg 사용 (ttm_resource 아님) -> ttm shim 불필요.
타깃: GCN4 (Polaris 4.7+) ~ RDNA1 (Navi).
