import numpy as np

def convert_q_to_zmq_str(franka_q=None, allegro_q=None, precision=6, cmd_type='ee'):
    zmq_str = ''
    if franka_q is not None:
        franka_q_1d = np.squeeze(franka_q)
        assert franka_q_1d.shape == (7,)
        franka_str = ','.join(map(str, franka_q_1d))
        zmq_str += franka_str
    zmq_str += ';'
    if allegro_q is not None:
        allegro_q_1d = np.squeeze(allegro_q)
        assert allegro_q_1d.shape == (16,)
        allegro_str = ','.join(map(str, allegro_q_1d))
        zmq_str += allegro_str
    return zmq_str + ':' + cmd_type

def convert_zmq_str_to_q(zmq_str):
    msg_str, cmd_type = zmq_str.split(':')
    franka_str, allegro_str = msg_str.split(';')
    franka_q = None
    allegro_q = None
    if len(franka_str) > 0:
        franka_q = np.fromstring(franka_str, dtype=float, sep=',')
    if len(allegro_str) > 0:
        allegro_q = np.fromstring(allegro_str, dtype=float, sep=',')
    return franka_q, allegro_q, cmd_type

def convert_allegro_q_to_zmq_str(allegro_q, precision=6):
    allegro_q_1d = np.squeeze(allegro_q)
    assert allegro_q_1d.shape == (16,)
    allegro_str = ','.join(map(str, allegro_q_1d))
    return allegro_str
