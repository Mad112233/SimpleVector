# SimpleVector

Учебный проект. Аналог std::vector на C++.
В основе SimpleVector — массив в динамической памяти.
Сам контейнер хранит лишь адрес начала массива, а также информацию о его текущем размере и вместимости.
Когда новый размер превышает текущую вместимость, SimpleVector создаёт новый массив большего размера в
динамической памяти, куда копирует элементы исходного массива и инициализирует остальные
элементы значением по умолчанию.
Очистку выполняет метод Clear.

Стандарт ISO C++ 20.
