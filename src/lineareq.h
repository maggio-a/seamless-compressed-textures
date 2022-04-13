#ifndef LINEAREQ_H
#define LINEAREQ_H

#endif // LINEAREQ_H

#include <iostream>
#include <map>
#include <vector>

#include "vec3.h"



// A linear expression: SUM_i{ a[i] * x[i] } + b
//  also used as linear expressions
struct LinearExp{
    std::map<int, scalar> terms; // i --> a[i]
    scalar b;

    scalar evaluateFor( const std::vector<scalar> & vars ) const {
        scalar res = b;
        for (auto& t : terms) res += t.second * vars[t.first];
        return res;
    }

    void print() const{
        for (const auto& t : terms) std::cout << t.second << "*x[" << t.first << "] + ";
        std::cout<<b;
    }

    /* basic linear expressions...*/

    LinearExp() : b(0) {}

    LinearExp(int vari ) : b(0) {
        terms[vari] = scalar(1);
    }

    LinearExp(scalar c ) : b(c) {}

    /* in=place operators */
    void operator *= (scalar k){ b *= k; for (auto& t : terms) t.second *= k; }
    void operator /= (scalar k){ b /= k; for (auto& t : terms) t.second /= k; }
    void operator += (scalar c){ b += c; }
    void operator -= (scalar c){ b -= c; }
    void operator += (const LinearExp & ex){ b += ex.b; for (const auto& t : ex.terms) terms[ t.first ] += t.second;}
    void operator -= (const LinearExp & ex){ b -= ex.b; for (const auto& t : ex.terms) terms[ t.first ] -= t.second;}
    void flip() { for (auto& t : terms) t.second = - t.second; b = -b; }

    /* out-of-place operators */
    LinearExp operator - () const{ LinearExp res = *this; res.flip(); return res; }
    LinearExp operator + (const LinearExp & other) const { LinearExp res = *this; res += other; return res; }
    LinearExp operator - (const LinearExp & other) const { LinearExp res = *this; res -= other; return res; }
    LinearExp operator ==(const LinearExp & other) const { LinearExp res = *this; res -= other; return res; }
    LinearExp operator - (scalar c) const { LinearExp res = *this; res -= c; return res; }
    LinearExp operator + (scalar c) const { LinearExp res = *this; res += c; return res; }
    LinearExp operator * (scalar k) const { LinearExp res = *this; res *= k; return res; }
    LinearExp operator / (scalar k) const { LinearExp res = *this; res /= k; return res; }
    LinearExp operator ==(scalar c) const { LinearExp res = *this; res -= c; return res; }

    bool isInvertible() const { return (terms.size() == 1) && (std::abs((terms.begin()->second)) < 1e-4); }

};

inline LinearExp mix(LinearExp a, LinearExp b, scalar t) {
    return a * (1 - t) + b * t;
}

/* */
inline LinearExp zero() { return LinearExp(); }
inline LinearExp constant(scalar c) { return LinearExp(c); }
inline LinearExp variable(int i) { return LinearExp(i); }

/* commuativity...*/
inline LinearExp operator * (scalar k, const LinearExp &a ) { return a*k;}
inline LinearExp operator + (scalar k, const LinearExp &a ) { return a+k;}
inline LinearExp operator - (scalar k, const LinearExp &a ) { return -a+k;}
inline LinearExp operator ==(scalar k, const LinearExp &a ) { return a==k;}




// a vec3 of linear expressions
struct LinearVec3{
    LinearExp x,y,z;

    LinearVec3( LinearExp _x, LinearExp _y, LinearExp _z): x(_x), y(_y), z(_z) {}
    LinearVec3(){}

    vec3 evaluateFor( const std::vector<scalar> & vars ) const {
        return vec3( x.evaluateFor(vars),  y.evaluateFor(vars),  z.evaluateFor(vars) );
    }

    /* in place operators */
    void operator += (const LinearVec3 &other){
        x += other.x;
        y += other.y;
        z += other.z;
    }

    void operator -= (const LinearVec3 &other){
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    void operator += (const vec3 &other){
        x += getx(other);
        y += gety(other);
        z += getz(other);
    }

    void operator -= (const vec3 &other){
        x -= getx(other);
        y -= gety(other);
        z -= getz(other);
    }

    void operator *= (scalar k){
        x *= k;
        y *= k;
        z *= k;
    }

    void operator /= (scalar k){
        x /= k;
        y /= k;
        z /= k;
    }

    void flip(){
        x.flip();
        y.flip();
        z.flip();
    }

    /* out-of-place operators */
    LinearVec3 operator + (const LinearVec3 &other) const { LinearVec3 res = *this; res += other; return res; }
    LinearVec3 operator - (const LinearVec3 &other) const { LinearVec3 res = *this; res -= other; return res; }
    LinearVec3 operator ==(const LinearVec3 &other) const { LinearVec3 res = *this; res -= other; return res; }
    LinearVec3 operator + (const vec3 &other) const { LinearVec3 res = *this; res += other; return res; }
    LinearVec3 operator - (const vec3 &other) const { LinearVec3 res = *this; res -= other; return res; }
    LinearVec3 operator ==(const vec3 &other) const { LinearVec3 res = *this; res -= other; return res; }
    LinearVec3 operator - () const { LinearVec3 res = *this; res.flip(); return res; }
    LinearVec3 operator * (scalar k) const { LinearVec3 res = *this; res*=k; return res; }
    LinearVec3 operator / (scalar k) const { LinearVec3 res = *this; res/=k; return res; }

    /* swizzle */
    LinearVec3 zxy() const {
        return LinearVec3( z,x,y );
    }

};



inline LinearExp dot ( LinearVec3 a, vec3 b ) { return a.x*getx(b) + a.y*gety(b) + a.z*getz(b); }
inline LinearVec3 cross ( LinearVec3 a, vec3 b ){
    return LinearVec3( a.y*getz(b) - a.z*gety(b) ,
                       a.z*getx(b) - a.x*getz(b) ,
                       a.x*gety(b) - a.y*getx(b) );
}

inline LinearVec3 mix(LinearVec3 a, LinearVec3 b, scalar t) {
    return a * (1 - t) + b * t;
}

inline LinearVec3 operator * ( LinearExp a,  vec3 b  ) {
    return LinearVec3(
                a * getx(b),
                a * gety(b),
                a * getz(b) );
}

/* (anti)-commutativity */
inline LinearExp dot ( vec3 b , LinearVec3 a) { return dot(a,b); }
inline LinearVec3 cross ( vec3 b , LinearVec3 a) { return cross(a,-b); }
inline LinearVec3 operator + ( vec3 b , LinearVec3 a) { return  a+b; }
inline LinearVec3 operator - ( vec3 b , LinearVec3 a) { return -a+b; }
inline LinearVec3 operator * ( scalar b , LinearVec3 a) { return a*b; }
inline LinearVec3 operator * ( vec3 b , LinearExp a) { return a*b; }


struct LinearMat3{
    LinearVec3 x,y,z; // columns

    LinearMat3( LinearVec3 _x, LinearVec3 _y, LinearVec3 _z): x(_x), y(_y), z(_z) {}
    LinearMat3(){}

    mat3 evaluateFor( const std::vector<scalar> & vars ) const {
        return mat3( x.evaluateFor(vars),  y.evaluateFor(vars),  z.evaluateFor(vars) );
    }

    /* in place operators */
    void operator += (const LinearMat3 &other){
        x += other.x;
        y += other.y;
        z += other.z;
    }

    void operator -= (const LinearMat3 &other){
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    LinearVec3 operator * (const vec3 &b){
        return x*b.x + y*b.y + z*b.z;
    }

    void operator += (const mat3 &other){
        x += getx(other);
        y += gety(other);
        z += getz(other);
    }

    void operator -= (const mat3 &other){
        x -= getx(other);
        y -= gety(other);
        z -= getz(other);
    }

    void operator /= (scalar k){
        x /= k;
        y /= k;
        z /= k;
    }

    void flip(){
        x.flip();
        y.flip();
        z.flip();
    }

    void transpose(){
        std::swap( x.y, y.x );
        std::swap( y.z, z.y );
        std::swap( z.x, x.z );
    }

    /* out-of-place operators */
    LinearMat3 operator + (const LinearMat3 &other) const { LinearMat3 res = *this; res += other; return res; }
    LinearMat3 operator - (const LinearMat3 &other) const { LinearMat3 res = *this; res -= other; return res; }
    LinearMat3 operator ==(const LinearMat3 &other) const { LinearMat3 res = *this; res -= other; return res; }
    LinearMat3 operator + (const mat3 &other) const { LinearMat3 res = *this; res += other; return res; }
    LinearMat3 operator - (const mat3 &other) const { LinearMat3 res = *this; res -= other; return res; }
    LinearMat3 operator ==(const mat3 &other) const { LinearMat3 res = *this; res -= other; return res; }
    LinearMat3 operator - () const { LinearMat3 res = *this; res.flip(); return res; }
    LinearMat3 operator / (scalar k) const { LinearMat3 res = *this; res/=k; return res; }

};

/* (anti)-commutativity */
inline LinearMat3 operator + ( mat3 b , LinearMat3 a) { return  a+b; }
inline LinearMat3 operator - ( mat3 b , LinearMat3 a) { return -a+b; }
inline LinearVec3 operator * ( vec3 b , LinearMat3 a) {
    return LinearVec3( dot(a.x,b), dot(a.y,b), dot(a.z,b) );
}

/* LinearVec products with constant mat3 */
inline LinearVec3 operator * ( LinearVec3 b , mat3 a) {
    return LinearVec3( dot(a[0],b), dot(a[1],b), dot(a[2],b) );
}
inline LinearVec3 operator * ( mat3 a , LinearVec3 b ) {
    return a[0]*b.x + a[1]*b.y + a[2]*b.z;
}


// a vec2 of linear expressions
struct LinearVec2{
    LinearExp x,y;

    LinearVec2( LinearExp _x, LinearExp _y): x(_x), y(_y) {}
    LinearVec2(){}

    vec2 evaluateFor( const std::vector<scalar> & vars ) const {
        return vec2( x.evaluateFor(vars),  y.evaluateFor(vars) );
    }


    void operator += (const LinearVec2 &other){
        x += other.x;
        y += other.y;
    }

    void operator -= (const LinearVec2 &other){
        x -= other.x;
        y -= other.y;
    }

    void operator += (const vec2 &other){
        x += getx(other);
        y += gety(other);
    }

    void operator -= (const vec2 &other){
        x -= getx(other);
        y -= gety(other);
    }

    void operator *= (scalar k){
        x *= k;
        y *= k;
    }

    void operator /= (scalar k){
        x *= k;
        y *= k;
    }


    void flip(){
        x.flip();
        y.flip();
    }

    /* out-of-place operators */
    LinearVec2 operator + (const LinearVec2 &other) const { LinearVec2 res = *this; res += other; return res; }
    LinearVec2 operator - (const LinearVec2 &other) const { LinearVec2 res = *this; res -= other; return res; }
    LinearVec2 operator ==(const LinearVec2 &other) const { LinearVec2 res = *this; res -= other; return res; }
    LinearVec2 operator + (const vec2 &other) const { LinearVec2 res = *this; res += other; return res; }
    LinearVec2 operator - (const vec2 &other) const { LinearVec2 res = *this; res -= other; return res; }
    LinearVec2 operator ==(const vec2 &other) const { LinearVec2 res = *this; res -= other; return res; }
    LinearVec2 operator - () const { LinearVec2 res = *this; res.flip(); return res; }
    LinearVec2 operator * (scalar k) const { LinearVec2 res = *this; res*=k; return res; }
    LinearVec2 operator / (scalar k) const { LinearVec2 res = *this; res/=k; return res; }

};

inline LinearExp dot ( LinearVec2 a, vec2 b ) { return a.x*getx(b) + a.y*gety(b); }
inline LinearExp cross ( LinearVec2 a, vec2 b ){ return a.x*gety(b) - a.y*getx(b); }

inline LinearVec2 operator * ( LinearExp a,  vec2 b  ) {
    return LinearVec2( a * getx(b), a * gety(b) );
}

/* (anti)-commutativity */
inline LinearExp dot ( vec2 b , LinearVec2 a) { return dot(a,b); }
inline LinearExp cross ( vec2 b , LinearVec2 a) { return cross(a,-b); }
inline LinearVec2 operator + ( vec2 b , LinearVec2 a) { return a+b; }
inline LinearVec2 operator - ( vec2 b , LinearVec2 a) { return -a+b; }
inline LinearVec2 operator * ( scalar b , LinearVec2 a) { return a*b; }
inline LinearVec2 operator * ( vec2 b , LinearExp a) { return a*b; }





struct LinearEquationSet{
    int nvar = 0;
    std::vector< LinearExp > eq;

    void clear(){ eq.clear(); nvar=0; }

    void print() const {
        printShort();
        for (const LinearExp &e: eq) {
            e.print();
            std::cout << " = 0\n  ";
        }
        std::cout << "\n";
    }

    void printShort() const {
        std::cout << eq.size() << " equations on "<< nvar << " variables" << std::endl;;

    }

    // evaluates a solution in the least square sense
    scalar squaredErrorFor(const std::vector<scalar> & x){

        assert( (int)x.size() >= nvar  );

        scalar tot = 0;
        for (const LinearExp &e: eq) {
            scalar err = e.evaluateFor( x );
            tot += err*err;
        }
        return tot;
    }

    void initializeVars(std::vector<scalar> & x){
        x.resize(nvar, 0);
        for (const LinearExp &e : eq) {
            if (e.isInvertible()) {
                x[e.terms.begin()->first] = - (e.b / e.terms.begin()->second);
            }
        }

    }

    void addEquation( const LinearVec3& v ) {
        eq.push_back(v.x);
        eq.push_back(v.y);
        eq.push_back(v.z);
    }

    void addEquation( const LinearVec2& v ) {
        eq.push_back(v.x);
        eq.push_back(v.y);
    }

    void addEquation( const LinearExp& v ) {
        eq.push_back(v);
    }

    int newVar() {
        return nvar++;
    }
    LinearVec2 newLinearVec2() {
        int v0 = newVar();
        int v1 = newVar();
        return LinearVec2( v0, v1 );
    }
    LinearVec3 newLinearVec3() {
        int v0 = newVar();
        int v1 = newVar();
        int v2 = newVar();
        return LinearVec3( v0, v1, v2 );
    }
    LinearMat3 newLinearMat3() {
        LinearVec3 v0 = newLinearVec3();
        LinearVec3 v1 = newLinearVec3();
        LinearVec3 v2 = newLinearVec3();
        return LinearMat3( v0, v1, v2 );
    }

    /* returns false if the system is underdetermined */
    bool solve( std::vector<scalar> & x );

    /*
    void addEquationsForTriangle( int u0i, int u1i, int u2i,
                                 vec3 p0, vec3 p1, vec3 p2,
                                 const Jacobian &J,
                                  scalar weightA,scalar weightB, scalar weightC){

        LinearVec2 u0 = LinearVec2( variable( u0i ), variable( u0i + 1 ) );
        LinearVec2 u1 = LinearVec2( variable( u1i ), variable( u1i + 1 ) );
        LinearVec2 u2 = LinearVec2( variable( u2i ), variable( u2i + 1 ) );

        p1-=p0;
        p2-=p0;

        u1-=u0;
        u2-=u0;

        scalar area3D = length( cross(p1,p2 ) );
        scalar area2D = area3D / J.areaMult();

        // the current grandient (approx as a Linear function of the vars)
        LinearVec3 u, v;
        u = p1*u2.y - p2*u1.y;
        v = p2*u1.x - p1*u2.x ;


        if (weightA!=0) {
            addEquation( weightA * ( dot(u,J.u) == area2D) ); // "u and v must be unit length" (shear)
            addEquation( weightA * ( dot(v,J.v) == area2D) );
        }

        if (weightB!=0) {
            addEquation( weightB * (cross(v,J.n()) == u) ); // "please be conformal"
        }


        if (weightC!=0) {
            addEquation( weightC * (u == J.u*area2D) ); // "please rigidly assume the hammered dirs"
            addEquation( weightC * (v == J.v*area2D) );
        }
    }



    void addEquationsForTriangleLSCM( int u0i, int u1i, int u2i,
                                 vec3 p0, vec3 p1, vec3 p2 )
    {
        LinearVec2 u0 = LinearVec2( variable( u0i ), variable( u0i+1 ) );
        LinearVec2 u1 = LinearVec2( variable( u1i ), variable( u1i+1 ) );
        LinearVec2 u2 = LinearVec2( variable( u2i ), variable( u2i+1 ) );

        p1-=p0;
        p2-=p0;

        u1-=u0;
        u2-=u0;

        LinearVec3 u, v;
        u = p1*u2.y - p2*u1.y;
        v = p2*u1.x - p1*u2.x ;

        vec3 n = normalize( cross(p1,p2) );

        addEquation( cross(v,n) == u );

    }*/

    void setAsTest2(){

        clear();

        nvar = 2;

        auto x0 = variable( 0 );
        auto x1 = variable( 1 );

        addEquation( 3*x0 + 5*x1 == 0 );

    }

    void setAsTest1(){

        clear();
        int xi = newVar();
        int yi = newVar();

        LinearExp e0,e1;

        e0.terms[ xi ] = 3.0;
        e0.terms[ yi ] = 2.0;
        e0.b = -12;

        e1.terms[ xi ] = 10.0;
        e1.b = -14;

        e1 += e0;

        eq.push_back( e0 );
        eq.push_back( e1 );
        eq.push_back( 100*e1+50*e0 ); // add a third equation, as a linear combo of the other two

    }

};




inline void unitTest00(){
    LinearEquationSet set;

    set.setAsTest1();

    std::vector<scalar> aSolution;
    set.solve( aSolution );
    std::cout << aSolution[0] << " "<< aSolution[1]  << "\n";;

    set.print();
    std::cout << "ERROR: "<< set.squaredErrorFor( aSolution ) <<"\n";

}
