import math
class Quat:
    """
        Quaternion Class
        >>> roll = 30
        >>> pitch = 45
        >>> heading = 30
        >>> Q1 = Quat(0,0,1, roll) * Quat(1,0,0, pitch) * \
               Quat(0,1,0, heading)
        >>> Q1.gl_matrix()
        [0.57322332027622536, 0.73919890801437749, \
        -0.3535533898605015, 0.0, -0.3535533898605015, \
        0.61237244705783134, 0.70710677171312086, 0.0, \
        0.73919890801437749, -0.28033009198934633, \
        0.61237244705783134, 0.0, 0, 0, 0, 1.0]
    """
    def __init__(self, *args, **kwargs):
        if 'w' in kwargs:
            self.w = kwargs['w']
            self.x = kwargs['x']
            self.y = kwargs['y']
            self.z = kwargs['z']
        elif len(args)==4:
            angle = (float(args[3]) / 180.) * 3.1415926
            result = math.sin(angle / 2.)
            self.w = math.cos(angle / 2.)
            self.x = float(args[0]) * result
            self.y = float(args[1]) * result
            self.z = float(args[2]) * result
        else:
            self.w = 1.
            self.x = 0.
            self.y = 0.
            self.z = 0.

    def gl_matrix(self):
        y=self.y
        x=self.x
        z=self.z
        w=self.w
        x2 = x*x
        y2 = y*y
        z2 = z*z
        xy = x*y
        xz = x*z
        yz = y*z
        wx = w*x
        wy = w*y
        wz = w*z
        
        """
           Returns OpenGL compatible modelview matrix
        """
        mat = [
            1.0 - 2.0 * (y2 + z2),
            2.0 * (xy - wz),
            2.0 * (xz + wy),
            0.0,
            2.0 * (xy + wz),
            1.0 - 2.0 * (x2 + z2),
            2.0 * (yz - wx),
            0.0,
            2.0 * (xz - wy),
            2.0 * (yz + wx),
            1.0 - 2.0 * (x2 + y2),
            0.0,
            0.0,
            0.0,
            0.0,
            1.0]
        return mat
    
    def __mul__(self, b):
        r = Quat()
        r.w = self.w*b.w - self.x*b.x - self.y*b.y - self.z*b.z
        r.x = self.w*b.x + self.x*b.w + self.y*b.z - self.z*b.y
        r.y = self.w*b.y + self.y*b.w + self.z*b.x - self.x*b.z
        r.z = self.w*b.z + self.z*b.w + self.x*b.y - self.y*b.x
        return(r)

    def rotate(self, vector):
        temp_quat_v = Quat(x=vector[0], y=vector[1], z=vector[2], w=0.)
        temp_quat_m = self * temp_quat_v * self.inverse()

        return (temp_quat_m.x, temp_quat_m.y, temp_quat_m.z)

    def inverse(self):
        conj = self.conjugate()
        inv = conj.scale(1/self.norm())
        return inv

    def conjugate(self):
        return Quat(x=-self.x, y=-self.y, z=-self.z, w=self.w)

    def scale(self, s):
        return Quat(x=self.x*s, y=self.y*s, z=self.z*s, w=self.w*s);

    def norm(self):
        return (self.w**2 + self.x**2 + self.y**2 + self.z**2);