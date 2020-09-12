//
// Created by neputin on 12.09.2020.
//

#ifndef CAE_MATH_H
#define CAE_MATH_H

inline bool operator<(sf::Vector2i f, sf::Vector2i s)
{
    return (f.x < s.x) && (f.y < s.y);
}

inline bool operator>(sf::Vector2i f, sf::Vector2i s)
{
    return !(f < s);
}

//template<class T>
//sf::Rect<T> round(sf::Rect<T> value)
//{
//	return sf::Rect <T>{round(value.left), round(value.top), round(value.width), round(value.height)};
//}

template<class T>
inline sf::Vector2<T> abs(sf::Vector2<T> value)
{
    return sf::Vector2<T>{std::abs(value.x), std::abs(value.y)};
}

#endif //CAE_MATH_H
