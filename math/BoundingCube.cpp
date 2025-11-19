#include "math.hpp"

BoundingCube::BoundingCube() : AbstractBoundingBox(glm::vec3(0)) {
	this->length = 0;
}

BoundingCube::BoundingCube(glm::vec3 min, float length) : AbstractBoundingBox(min) {
	this->length = length;
}

glm::vec3 BoundingCube::getMax() const {
    return getMin()+glm::vec3(length);
}

float BoundingCube::getMaxX() const {
    return getMinX() + length;
}

float BoundingCube::getMaxY() const {
    return getMinY() + length;
}

float BoundingCube::getMaxZ() const {
    return getMinZ() + length;
}

glm::vec3 BoundingCube::getLength() const {
    return glm::vec3(length);
}

float BoundingCube::getLengthX() const {
    return length;
}

float BoundingCube::getLengthY() const {
    return length;
}

float BoundingCube::getLengthZ() const {
    return length;
}

void BoundingCube::setLength(float l) {
    this->length = l;
}

BoundingCube BoundingCube::getChild(int i) const {
	float newLength = 0.5*getLengthX();
    return BoundingCube(getMin() + newLength * AbstractBoundingBox::getShift(i), newLength);
}

bool BoundingCube::operator<(const BoundingCube& other) const {
    if (getMinX() != other.getMinX()) return getMinX() < other.getMinX();
    if (getMinY() != other.getMinY()) return getMinY() < other.getMinY();
    if (getMinZ() != other.getMinZ()) return getMinZ() < other.getMinZ();
    return length < other.length;
}

bool BoundingCube::operator==(const BoundingCube& other) const {
    return getMinX() == other.getMinX() 
        && getMinY() == other.getMinY()
        && getMinZ() == other.getMinZ() 
        && length == other.length;
}

bool BoundingCube::overlaps1D(float aMin, float aMax, float bMin, float bMax) const
{
    return (aMin < bMax) && (bMin < aMax);  // strict overlap
}

bool BoundingCube::overlapsX(const BoundingCube &o) const {
    return overlaps1D(getMinX(), getMaxX(), o.getMinX(), o.getMaxX());
}

bool BoundingCube::overlapsY(const BoundingCube &o) const {
    return overlaps1D(getMinY(), getMaxY(), o.getMinY(), o.getMaxY());
}

bool BoundingCube::overlapsZ(const BoundingCube &o) const {
    return overlaps1D(getMinZ(), getMaxZ(), o.getMinZ(), o.getMaxZ());
}

bool BoundingCube::isFaceAdjacent(const BoundingCube &o) const
{
    bool xTouch = (getMaxX() == o.getMinX()) || (getMinX() == o.getMaxX());
    bool yTouch = (getMaxY() == o.getMinY()) || (getMinY() == o.getMaxY());
    bool zTouch = (getMaxZ() == o.getMinZ()) || (getMinZ() == o.getMaxZ());

    // Must touch on exactly ONE axis → face adjacency
    int touches = (int)xTouch + (int)yTouch + (int)zTouch;
    if (touches != 1)
        return false;

    // For the other two axes: they must overlap
    if (xTouch)
        return overlapsY(o) && overlapsZ(o);
    if (yTouch)
        return overlapsX(o) && overlapsZ(o);
    if (zTouch)
        return overlapsX(o) && overlapsY(o);

    return false;
}

bool BoundingCube::isNeighbor(const BoundingCube &o) const
{
    bool larger = getLengthX() > o.getLengthX();

    if (!larger)
        return isFaceAdjacent(o);

    // larger cube: not a neighbor, but must descend only if overlapping
    return intersects(o);
}

void BoundingCube::setMinX(float v){
    this->min.x = v;
}

void BoundingCube::setMinY(float v){
    this->min.y = v;
}

void BoundingCube::setMinZ(float v){
    this->min.z = v;
}

void BoundingCube::setMaxX(float v){
    this->min.x = v-length;
}

void BoundingCube::setMaxY(float v){
    this->min.y = v-length;
}

void BoundingCube::setMaxZ(float v){
    this->min.z = v-length;
}