->make
->./trace_processed in_path, ref_path, out_path, rnode_num, vnode_num, from_node, to_node, ratio

in_path    : ��Ҫ�����trace
ref_path   : �����ṩrequest id��trace��ͨ���޸�request id���ø������䵽����vnode�ϣ�
out_path   : ����õ�trace��ַ����
rnode_num  : ʵ�ڵ���
vnode_num  : ÿ��ʵ�ڵ����ڵ���
from_node  :  
to_node    : ������ from_node�� vnode*ratio ����ڵ��ϵ� request��id �޸�Ϊ����to_node��
ratio      : 

Example: ./trace_processor wikiaa wikiad wikiaa_processed 4 40 0 1 0.5
wikiaa��ʵ�ڵ�0��40����ڵ㣬���������е�20��40*0.5������ڵ��request��id��Ϊ����ʵ�ڵ�1�ϣ�wikiad�����ṩ�޸�������Ҫ��request id
